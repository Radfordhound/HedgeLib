using HedgeLib.IO;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Xml.Linq;

namespace HedgeLib.Sets
{
    public class SetData : FileBase
    {
        //Variables/Constants
        public List<SetObject> Objects = new List<SetObject>();
        public string Name = null;

        //Methods
        public override void Load(string filePath)
        {
            Load(filePath, null);
        }

        public override void Load(Stream fileStream)
        {
            Load(fileStream, null);
        }

        public virtual void Load(string filePath,
            Dictionary<string, SetObjectType> objectTemplates)
        {
            using (var fileStream = File.OpenRead(filePath))
            {
                Load(fileStream, objectTemplates);
            }
        }

        public virtual void Load(Stream fileStream,
            Dictionary<string, SetObjectType> objectTemplates)
        {
            throw new NotImplementedException();
        }

		public void ImportXML(string filePath)
		{
			using (var fileStream = File.OpenRead(filePath))
			{
				ImportXML(fileStream);
			}
		}

		public void ImportXML(Stream fileStream)
		{
			//Load XML and add loaded data to set data
			var xml = XDocument.Load(fileStream);
			uint objID = 0; //For Object elements with no ID attribute.

			foreach (var objElem in xml.Root.Elements("Object"))
			{
				//Generate Object
				var typeAttr = objElem.Attribute("type");
				var objIDAttr = objElem.Attribute("id");
				if (typeAttr == null) continue;

				var obj = new SetObject()
				{
					ObjectType = typeAttr.Value,
					ObjectID = (objIDAttr == null) ?
						objID : Convert.ToUInt32(objIDAttr.Value),
				};

				//Assign CustomData to Object
				var customDataElem = objElem.Element("CustomData");
				if (customDataElem != null)
				{
					foreach (var customData in customDataElem.Elements())
					{
						obj.CustomData.Add(customData.Name.LocalName,
							LoadParam(customData));
					}
				}

				//Assign Parameters to Object
				var parametersElem = objElem.Element("Parameters");
				if (parametersElem != null)
				{
					foreach (var paramElem in parametersElem.Elements())
					{
						obj.Parameters.Add(LoadParam(paramElem));
					}
				}

				//Assign Transforms to Object
				var transformsElem = objElem.Element("Transforms");
				if (transformsElem != null)
				{
					var transforms = transformsElem.Elements("Transform");
					int transformCount = transforms.Count();

					if (transformCount > 0)
					{
						uint i = 0;
						obj.Children = new SetObjectTransform[transformCount - 1];

						foreach (var transformElem in transforms)
						{
							var transform = LoadTransform(transformElem);
							if (i > 0)
							{
								obj.Children[i - 1] = transform;
							}
							else
							{
								obj.Transform = transform;
							}

							++i;
						}
					}
				}

				++objID;
				Objects.Add(obj);
			}

			//Sub-Methods
			SetObjectParam LoadParam(XElement paramElem)
			{
				var dataTypeAttr = paramElem.Attribute("type");
				if (dataTypeAttr == null) return null;

				var dataType = Types.GetTypeFromString(dataTypeAttr.Value);
				object data = null;

				if (dataType == typeof(Vector3))
				{
					data = Helpers.XMLReadVector3(paramElem);
				}
				else if (dataType == typeof(Vector4))
				{
					data = Helpers.XMLReadVector4(paramElem);
				}
				else if (dataType == typeof(Quaternion))
				{
					data = Helpers.XMLReadQuat(paramElem);
				}
				else
				{
					data = Convert.ChangeType(paramElem.Value, dataType);
				}

				return new SetObjectParam(dataType, data);
			}

			SetObjectTransform LoadTransform(XElement elem)
			{
				var posElem = elem.Element("Position");
				var rotElem = elem.Element("Rotation");
				var scaleElem = elem.Element("Scale");

				return new SetObjectTransform()
				{
					Position = Helpers.XMLReadVector3(posElem),
					Rotation = Helpers.XMLReadQuat(rotElem),
					Scale = Helpers.XMLReadVector3(scaleElem)
				};
			}
		}

		public void ExportXML(string filePath,
			Dictionary<string, SetObjectType> objectTemplates = null)
		{
			using (var fileStream = File.OpenWrite(filePath))
			{
				ExportXML(fileStream, objectTemplates);
			}
		}

		public void ExportXML(Stream fileStream,
			Dictionary<string, SetObjectType> objectTemplates = null)
		{
			//Convert to XML file and save
			var rootElem = new XElement("SetData");

			foreach (var obj in Objects)
			{
				//Generate Object Element
				var objElem = new XElement("Object");
				var typeAttr = new XAttribute("type", obj.ObjectType);
				var objIDAttr = new XAttribute("id", obj.ObjectID);

				//Generate CustomData Element
				var customDataElem = new XElement("CustomData");
				foreach (var customData in obj.CustomData)
				{
					customDataElem.Add(GenerateParamElement(
						customData.Value, customData.Key));
				}

				//Generate Parameters Element
				var paramsElem = new XElement("Parameters");
				var template = objectTemplates?[obj.ObjectType];

				for (int i = 0; i < obj.Parameters.Count; ++i)
				{
					paramsElem.Add(GenerateParamElement(obj.Parameters[i],
						template?.Parameters[i].Name));
				}

				//Generate Transforms Element
				var transformElem = GenerateTransformElement(obj.Transform);
				var transformsElem = new XElement("Transforms", transformElem);

				foreach (var transform in obj.Children)
				{
					transformsElem.Add(GenerateTransformElement(transform));
				}

				//Add all of this to the XDocument
				objElem.Add(typeAttr, objIDAttr, customDataElem,
					paramsElem, transformsElem);
				rootElem.Add(objElem);
			}

			var xml = new XDocument(rootElem);
			xml.Save(fileStream);

			//Sub-Methods
			XElement GenerateParamElement(
				SetObjectParam param, string name = "Parameter")
			{
				var dataType = param.DataType;
				var dataTypeAttr = new XAttribute("type", dataType.Name);
				var elem = new XElement((string.IsNullOrEmpty(name)) ?
					"Parameter" : name, dataTypeAttr);

				if (dataType == typeof(Vector3))
				{
					Helpers.XMLWriteVector3(elem, (Vector3)param.Data);
				}
				else if (dataType == typeof(Vector4) || dataType == typeof(Quaternion))
				{
					Helpers.XMLWriteVector4(elem, (Vector4)param.Data);
				}
				else
				{
					elem.Value = param.Data.ToString();
				}

				return elem;
			}

			XElement GenerateTransformElement(
				SetObjectTransform transform, string name = "Transform")
			{
				//Convert Position/Rotation/Scale into elements.
				var posElem = new XElement("Position");
				var rotElem = new XElement("Rotation");
				var scaleElem = new XElement("Scale");

				Helpers.XMLWriteVector3(posElem, transform.Position);
				Helpers.XMLWriteVector4(rotElem, transform.Rotation);
				Helpers.XMLWriteVector3(scaleElem, transform.Scale);

				//Add elements to new transform element and return it.
				return new XElement(name, posElem, rotElem, scaleElem);
			}
		}

        //Temporary solution for exporting Generations XML file. Delete when proper solution is developed.
        public void GensExportXML(string filePath,
            Dictionary<string, SetObjectType> objectTemplates = null)
        {
            using (var fileStream = File.OpenWrite(filePath))
            {
                GensExportXML(fileStream, objectTemplates);
            }
        }

        public void GensExportXML(Stream fileStream,
            Dictionary<string, SetObjectType> objectTemplates = null)
        {
            //Convert to XML file and save
            var rootElem = new XElement("SetObject");

            foreach (var obj in Objects)
            {
                //Generate Object Element
                //Messy rename to prevent error caused by element name starting with a number.
                if (obj.ObjectType == "1UP")
                {
                    obj.ObjectType = "Item";
                }
                var objElem = new XElement(obj.ObjectType);

                //Generate CustomData Element
                //Messy use RangeOut value as Range value.
                foreach (var customData in obj.CustomData)
                {
                    if (customData.Key == "RangeOut")
                    {
                        objElem.Add(GenerateParamElementGens(
                            customData.Value, "Range"));
                    }

                }

                //Generate Parameters Element
                var template = objectTemplates?[obj.ObjectType];

                for (int i = 0; i < obj.Parameters.Count; ++i)
                {
                    var name = template?.Parameters[i].Name;
                    objElem.Add(GenerateParamElementGens(obj.Parameters[i],
                        template?.Parameters[i].Name));
                }

                //Generate Transforms Elements
                objElem.Add(GeneratePositionElement(obj.Transform));
                objElem.Add(GenerateRotationElement(obj.Transform));

                //Generate ID Element
                var objIDAttr = new XElement("SetObjectID", obj.ObjectID);
                objElem.Add(objIDAttr);

                ////Generate MultiSet Elements
                if (obj.Children.Length > 0)
                {
                    var multiElem = new XElement("MultiSetParam");

                    for (int i = 0; i < obj.Children.Length; ++i)
                    {
                        var childElem = new XElement("Element");
                        childElem.Add(new XElement("Index", i + 1));
                        childElem.Add(GeneratePositionElement(obj.Children[i]));
                        childElem.Add(GenerateRotationElement(obj.Children[i]));
                        multiElem.Add(childElem);
                    }
                    multiElem.Add(new XElement("BaseLine", 1));
                    multiElem.Add(new XElement("Direction", 0));
                    multiElem.Add(new XElement("Interval", 1));
                    multiElem.Add(new XElement("IntervalBase", 0));
                    multiElem.Add(new XElement("PositionBase", 0));
                    multiElem.Add(new XElement("RotationBase", 0));
                    objElem.Add(multiElem);
                }

                //Add all of this to the XDocument
                rootElem.Add(objElem);
            }

            var xml = new XDocument(rootElem);
            xml.Save(fileStream);

            //Sub-Methods
            XElement GenerateParamElementGens(
                SetObjectParam param, string name)
            {
                var dataType = param.DataType;
                var elem = new XElement((string.IsNullOrEmpty(name)) ?
                    "Parameter" : name);

                if (dataType == typeof(Vector3))
                {
                    Helpers.XMLWriteVector3(elem, (Vector3)param.Data);
                }
                else if (dataType == typeof(Vector4) || dataType == typeof(Quaternion))
                {
                    Helpers.XMLWriteVector4(elem, (Vector4)param.Data);
                }
                else
                {
                    elem.Value = param.Data.ToString();
                }

                return elem;
            }

            XElement GeneratePositionElement(
                SetObjectTransform transform, string name = "Transform")
            {
                //Convert Position into elements.
                var posElem = new XElement("Position");

                //Scaling
                transform.Position.X = (transform.Position.X / 10);
                transform.Position.Y = (transform.Position.Y / 10);
                transform.Position.Z = (transform.Position.Z / 10);

                Helpers.XMLWriteVector3(posElem, transform.Position);

                //Add elements to new position element and return it.
                return new XElement(posElem);
            }

            XElement GenerateRotationElement(
                SetObjectTransform transform, string name = "Transform")
            {
                //Convert Rotation into elements.
                var rotElem = new XElement("Rotation");

                Helpers.XMLWriteVector4(rotElem, transform.Rotation);

                //Add elements to new rotation element and return it.
                return new XElement(rotElem);
            }
        }
        //End of temp solution.
    }
}
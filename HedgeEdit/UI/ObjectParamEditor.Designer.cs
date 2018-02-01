namespace HedgeEdit.UI
{
    partial class ObjectParamEditor
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.okBtn = new System.Windows.Forms.Button();
            this.cancelBtn = new System.Windows.Forms.Button();
            this.valueTxtBx = new System.Windows.Forms.TextBox();
            this.descriptionLbl = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // okBtn
            // 
            this.okBtn.Location = new System.Drawing.Point(291, 105);
            this.okBtn.Name = "okBtn";
            this.okBtn.Size = new System.Drawing.Size(80, 30);
            this.okBtn.TabIndex = 0;
            this.okBtn.Text = "&OK";
            this.okBtn.UseVisualStyleBackColor = true;
            this.okBtn.Click += new System.EventHandler(this.OkBtn_Click);
            // 
            // cancelBtn
            // 
            this.cancelBtn.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.cancelBtn.Location = new System.Drawing.Point(205, 105);
            this.cancelBtn.Name = "cancelBtn";
            this.cancelBtn.Size = new System.Drawing.Size(80, 30);
            this.cancelBtn.TabIndex = 1;
            this.cancelBtn.Text = "&Cancel";
            this.cancelBtn.UseVisualStyleBackColor = true;
            // 
            // valueTxtBx
            // 
            this.valueTxtBx.Location = new System.Drawing.Point(13, 48);
            this.valueTxtBx.Name = "valueTxtBx";
            this.valueTxtBx.Size = new System.Drawing.Size(353, 26);
            this.valueTxtBx.TabIndex = 2;
            // 
            // descriptionLbl
            // 
            this.descriptionLbl.Dock = System.Windows.Forms.DockStyle.Fill;
            this.descriptionLbl.Location = new System.Drawing.Point(0, 0);
            this.descriptionLbl.Name = "descriptionLbl";
            this.descriptionLbl.Padding = new System.Windows.Forms.Padding(0, 10, 0, 0);
            this.descriptionLbl.Size = new System.Drawing.Size(378, 144);
            this.descriptionLbl.TabIndex = 3;
            this.descriptionLbl.Text = "Description";
            this.descriptionLbl.TextAlign = System.Drawing.ContentAlignment.TopCenter;
            // 
            // ObjectParamEditor
            // 
            this.AcceptButton = this.okBtn;
            this.AutoScaleDimensions = new System.Drawing.SizeF(9F, 20F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.White;
            this.CancelButton = this.cancelBtn;
            this.ClientSize = new System.Drawing.Size(378, 144);
            this.Controls.Add(this.valueTxtBx);
            this.Controls.Add(this.cancelBtn);
            this.Controls.Add(this.okBtn);
            this.Controls.Add(this.descriptionLbl);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "ObjectParamEditor";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Object Param Editor";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button okBtn;
        private System.Windows.Forms.Button cancelBtn;
        private System.Windows.Forms.TextBox valueTxtBx;
        private System.Windows.Forms.Label descriptionLbl;
    }
}
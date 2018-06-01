namespace HedgeCnvrsEditor
{
    partial class ParameterEditor
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
            this.dataBx = new System.Windows.Forms.TextBox();
            this.OkBtn = new System.Windows.Forms.Button();
            this.CancelBtn = new System.Windows.Forms.Button();
            this.dataLbl = new System.Windows.Forms.Label();
            this.typeBx = new System.Windows.Forms.TextBox();
            this.typeLbl = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // dataBx
            // 
            this.dataBx.Location = new System.Drawing.Point(12, 85);
            this.dataBx.MaxLength = 2147483647;
            this.dataBx.Multiline = true;
            this.dataBx.Name = "dataBx";
            this.dataBx.Size = new System.Drawing.Size(551, 84);
            this.dataBx.TabIndex = 0;
            this.dataBx.WordWrap = false;
            this.dataBx.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            // 
            // OkBtn
            // 
            this.OkBtn.Location = new System.Drawing.Point(407, 187);
            this.OkBtn.Name = "OkBtn";
            this.OkBtn.Size = new System.Drawing.Size(75, 23);
            this.OkBtn.TabIndex = 1;
            this.OkBtn.Text = "OK";
            this.OkBtn.UseVisualStyleBackColor = true;
            this.OkBtn.Click += new System.EventHandler(this.OkBtn_Click);
            // 
            // CancelBtn
            // 
            this.CancelBtn.Location = new System.Drawing.Point(488, 187);
            this.CancelBtn.Name = "CancelBtn";
            this.CancelBtn.Size = new System.Drawing.Size(75, 23);
            this.CancelBtn.TabIndex = 2;
            this.CancelBtn.Text = "Cancel";
            this.CancelBtn.UseVisualStyleBackColor = true;
            this.CancelBtn.Click += new System.EventHandler(this.CancelBtn_Click);
            // 
            // dataLbl
            // 
            this.dataLbl.AutoSize = true;
            this.dataLbl.Location = new System.Drawing.Point(13, 66);
            this.dataLbl.Name = "dataLbl";
            this.dataLbl.Size = new System.Drawing.Size(30, 13);
            this.dataLbl.TabIndex = 3;
            this.dataLbl.Text = "Data";
            // 
            // typeBx
            // 
            this.typeBx.Location = new System.Drawing.Point(12, 32);
            this.typeBx.Name = "typeBx";
            this.typeBx.Size = new System.Drawing.Size(551, 20);
            this.typeBx.TabIndex = 4;
            // 
            // typeLbl
            // 
            this.typeLbl.AutoSize = true;
            this.typeLbl.Location = new System.Drawing.Point(9, 9);
            this.typeLbl.Name = "typeLbl";
            this.typeLbl.Size = new System.Drawing.Size(57, 13);
            this.typeLbl.TabIndex = 5;
            this.typeLbl.Text = "Data Type";
            // 
            // ParameterEditor
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(575, 222);
            this.Controls.Add(this.typeLbl);
            this.Controls.Add(this.typeBx);
            this.Controls.Add(this.dataLbl);
            this.Controls.Add(this.CancelBtn);
            this.Controls.Add(this.OkBtn);
            this.Controls.Add(this.dataBx);
            this.MaximizeBox = false;
            this.MaximumSize = new System.Drawing.Size(591, 261);
            this.MinimizeBox = false;
            this.MinimumSize = new System.Drawing.Size(591, 261);
            this.Name = "ParameterEditor";
            this.ShowIcon = false;
            this.Text = "ParameterEditor";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox dataBx;
        private System.Windows.Forms.Button OkBtn;
        private System.Windows.Forms.Button CancelBtn;
        private System.Windows.Forms.Label dataLbl;
        private System.Windows.Forms.TextBox typeBx;
        private System.Windows.Forms.Label typeLbl;
    }
}
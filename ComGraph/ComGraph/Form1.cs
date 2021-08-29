using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;


using System.IO.Ports;
using System.Runtime.InteropServices;

namespace ComGraph
{
    public partial class ComGraph : Form
    {
        // Создаем объект для работы с портами
        SerialPort serialportUser = new SerialPort();

        //Настройки графика
        private static bool EnableDraw; // разрешение рисования
        private static int NumOfChannels; // Число каналов ( или число датчиков, измерения которых приходит по  UART)
        private static int DrawChannelFrom; // С какого канала начать рисовать
        private static int DrawNumofChannels; // Сколько каналов отрисовать
        private static int NumofByteOnechannel; // Сколько байт в одном канале
        private static int SyncrByte; // Байт синхронизации
        private static bool EnableSyncByte; // Есть ли байт синхронизации
        private static int BandwithOfGraph; // Ширина графика
        private static bool EnableBandwith; // Отрисовывать окном
        private static int MinScale; // Максимальное значение по Y
        private static int MaxScale; // минимальное значение по Y


        public ComGraph()
        {
            InitializeComponent();
        }

        //https://docs.microsoft.com/en-us/windows/win32/devio/wm-devicechange
        //https://coderoad.ru/16245706/%D0%9F%D1%80%D0%BE%D0%B2%D0%B5%D1%80%D1%8C%D1%82%D0%B5-%D0%BD%D0%B0%D0%BB%D0%B8%D1%87%D0%B8%D0%B5-%D1%81%D0%BE%D0%B1%D1%8B%D1%82%D0%B8%D0%B9-%D1%81%D0%BC%D0%B5%D0%BD%D1%8B-%D1%83%D1%81%D1%82%D1%80%D0%BE%D0%B9%D1%81%D1%82%D0%B2%D0%B0-add-remove

        // Отслеживаем отключение, подключение COM портов
        private struct DEV_BROADCAST_HDR
        {
            //отключаем предупреждения компилятора для ошибки 0649
            #pragma warning disable 0649
            internal UInt32 dbch_size;
            internal UInt32 dbch_devicetype;
            internal UInt32 dbch_reserved;
            //включаем предупреждения компилятора для ошибки 0649
            #pragma warning restore 0649
        };
        protected override void WndProc(ref Message m)
        {
            base.WndProc(ref m);
            if (m.Msg == 0x0219)                 //WM_DEVICECHANGE = 0x0219
            {
                DEV_BROADCAST_HDR dbh;
                switch ((int)m.WParam)
                {
                    case 0x8000: //DBT_DEVICEARRIVAL = 0x8000
                        dbh = (DEV_BROADCAST_HDR)Marshal.PtrToStructure(m.LParam, typeof(DEV_BROADCAST_HDR));
                        if (dbh.dbch_devicetype == 0x00000003) //DBT_DEVTYP_PORT = 0x00000003
                            getCOMports();
                        break;
                    case 0x8004: //DBT_DEVICEREMOVECOMPLETE = 0x8004                  
                        dbh = (DEV_BROADCAST_HDR)Marshal.PtrToStructure(m.LParam, typeof(DEV_BROADCAST_HDR));
                        if (dbh.dbch_devicetype == 0x00000003) //DBT_DEVTYP_PORT = 0x00000003
                            getCOMports();
                        break;
                }
                
                if (!serialportUser.IsOpen)
                {
                    buttonConnect.Text = "Connect";
                    toolStripStatus.Text = "Disconnect";
                    comboBoxPort.Enabled = true;
                    buttonPlotFromCOM.Enabled = false;
                }
                
            }
        }
        // Обработчик прерываний по приему 
        private void DataReceivedHandler(object sender,SerialDataReceivedEventArgs e)
        {
            SerialPort sp = (SerialPort)sender;
            int indata;
            try
            {
                indata = sp.ReadByte();
            }
            catch (Exception excUser)
            {
                return;
            }
            GraphDrawUser(indata);
        }



        private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void listBox1_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void label2_Click(object sender, EventArgs e)
        {

        }

        private void label3_Click(object sender, EventArgs e)
        {

        }

        private void label1_Click(object sender, EventArgs e)
        {

        }

        private void label4_Click(object sender, EventArgs e)
        {

        }

        private void buttonConnect_Click(object sender, EventArgs e)
        {
            
            if (serialportUser.IsOpen==false && buttonConnect.Text=="Disconnect")
            {
                buttonConnect.Text = "Connect";
                toolStripStatus.Text = "Disconnect";
                comboBoxPort.Enabled = true;
                buttonPlotFromCOM.Enabled = false;
                return;
            }

                // Если порт открыт, то закрываем его и меняем назначение кнопки на открытие порта
            if (serialportUser.IsOpen)
            {
                serialportUser.Close();
                buttonConnect.Text = "Connect";
                toolStripStatus.Text = "Disconnect";
                comboBoxPort.Enabled = true;
                buttonPlotFromCOM.Enabled = false;
            }
            else // иначе пытаемся подключиться к порту
            {
                try
                {
                    // настройки порта
                    serialportUser.PortName = comboBoxPort.SelectedItem.ToString();
                    serialportUser.BaudRate = int.Parse(comboBoxBaudRate.SelectedItem.ToString());
                    serialportUser.DataBits = int.Parse(comboBoxDataBits.SelectedItem.ToString());
                    serialportUser.Parity = (Parity)comboBoxParity.SelectedIndex;
                    serialportUser.StopBits = (StopBits)(comboBoxStopBits.SelectedIndex + 1);
                    serialportUser.ReadTimeout = (int)numericUpDownTimeout.Value * 1000;
                    serialportUser.WriteTimeout = (int)numericUpDownTimeout.Value * 1000;
                    serialportUser.Handshake = (Handshake)comboBoxFlowcontrol.SelectedIndex;
                    serialportUser.Encoding = Encoding.ASCII;
                    serialportUser.ReceivedBytesThreshold = 1;
                    serialportUser.DataReceived += new SerialDataReceivedEventHandler(DataReceivedHandler);

                    serialportUser.Open();
                }
                catch (Exception excUser)
                {   
                    MessageBox.Show(
                        "ERROR: can't open port:" + excUser.ToString(),
                        "ERROR",
                        MessageBoxButtons.OK,
                        MessageBoxIcon.Warning,
                        MessageBoxDefaultButton.Button1,
                        MessageBoxOptions.DefaultDesktopOnly);
                    toolStripStatus.Text = "ERROR: can't open port";
                    return;
                }
                toolStripStatus.Text = "Connect";
                buttonConnect.Text = "Disconnect";
                comboBoxPort.Enabled = false;
                buttonPlotFromCOM.Enabled = true;
            }
        }

        private void comboBoxParity_SelectedIndexChanged_1(object sender, EventArgs e)
        {

        }

        private void label5_Click(object sender, EventArgs e)
        {

        }

        private void label6_Click(object sender, EventArgs e)
        {

        }

        private void comboBoxPort_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void comboBoxDataBits_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void comboBoxBaudRate_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void comboBoxFlowcontrol_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void ComGraph_Load(object sender, EventArgs e)
        {
            // Ставим первую точку на графике, чтобы были видны клеточки
            chart1.Series[0].Points.AddXY(0, 1);
            // Не даем нажать кнопку построения графика по порту
            buttonPlotFromCOM.Enabled = false;
            // Вывод дефолтных параметров COM порта
            comboBoxDataBits.SelectedIndex = comboBoxDataBits.Items.IndexOf("8");
            comboBoxFlowcontrol.SelectedIndex = 0;
            comboBoxParity.SelectedIndex = 0;
            comboBoxStopBits.SelectedIndex = 0;
            comboBoxBaudRate.SelectedIndex = comboBoxBaudRate.Items.IndexOf("115200");
            // Выводим доступные COM порты
            getCOMports();
        }

        private void numericUpDownTimeout_ValueChanged(object sender, EventArgs e)
        {

        }

        private void comboBoxPort_MouseClick(object sender, MouseEventArgs e)
        {

        }

        private void comboBoxBaudRate_KeyPress(object sender, KeyPressEventArgs e)
        {
            // Выводим только цифры и backspace
            char number = e.KeyChar;

            if (!Char.IsDigit(number) && number != (char)ConsoleKey.Backspace)
            {
                e.Handled = true;
            }
        }


        //получаем список доступных COM портов в системе
        private void getCOMports()
        {
            //serialportUser.Close();
            string[] ports = SerialPort.GetPortNames();
            comboBoxPort.Items.Clear();
            
            ports = SerialPort.GetPortNames();
            comboBoxPort.Items.AddRange(ports);
            // Если есть доступные устройства, но ничего не выбрано
            if (comboBoxPort.Items.Count>0 && comboBoxPort.SelectedIndex<0)
            {
                comboBoxPort.SelectedIndex = 0;
            }
            else if(comboBoxPort.SelectedIndex < 0) //  Если нет устройств
            {
                comboBoxPort.Items.Add("Close");
                comboBoxPort.SelectedIndex = 0;
            }
        }

        private void chart1_Click(object sender, EventArgs e)
        {

        }

        private void openFileDialog1_FileOk(object sender, CancelEventArgs e)
        {

        }
        // Закгрузка файла с настрйоками построения графика
        private void buttonLoadFileSettings(object sender, EventArgs e)
        {
            if (openFileDialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                // Записываем в поле название файла
                textBoxLoadFileSettings.Text = openFileDialog.FileName;
                // Читам настройки из файла











    }
        }

        private void buttonPlotFromCOM_Click(object sender, EventArgs e)
        {
            if(EnableDraw==false)
            {
                chart1.Series[0].Points.Clear();
                EnableDraw = true;
            }
            else EnableDraw = false;
        }

        private void buttonPlotfile_Click(object sender, EventArgs e)
        {
            if (openFileDialogCSV.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                // Читам Данные из файла и строим на графике
            }
        }
        private int count = 0;
        private void GraphDrawUser(int DataFromCOM)
        {
            DateTime DateTimeUser = DateTime.Now;
            if (EnableDraw == true && serialportUser.IsOpen==true)
            {
                count++;
                if(count>1000)
                {
                    count = 0;
                }
                chart1.Invoke((MethodInvoker)delegate
                {
                    chart1.Series[0].Points.AddXY(count, DataFromCOM);
                });

            }
        }
    }
}

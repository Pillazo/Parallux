Imports System.Net
Imports System.Net.Sockets
Imports System.Text

Public Class Form1

    Dim SerialList As String() = System.IO.Ports.SerialPort.GetPortNames()
    Dim NMUDchanging As Boolean = False
    Dim TBChanging As Boolean = False
    Dim port As String
    Dim channelValues As New List(Of Integer)
    Dim channelAddresses As New List(Of Integer)
    Dim channels As Integer = 512
    Dim sceneNames As New List(Of String)
    Dim SceneChannels As New List(Of List(Of Integer))
    Dim sceneValues As New List(Of List(Of Integer))

    Dim ArtnetUDP As New UdpClient()
    Dim ArtnetPort As Integer = 6454
    Dim ANIP As IPAddress = IPAddress.Parse("10.90.30.255")
    Dim ANEP As New IPEndPoint(ANIP, ArtnetPort)

    Dim sACNPort As Integer = 5568
    Dim sACNUDP As New UdpClient(sACNPort)
    Dim sACNIP As IPAddress = IPAddress.Parse("239.255.0.1")
    Dim sACNIP2 As IPAddress = IPAddress.Parse("239.255.0.16")
    Dim sACNEP As New IPEndPoint(sACNIP, sACNPort)
    Dim localIP As IPAddress = IPAddress.Parse("10.0.0.80")
    Dim sACNActive As Boolean = False
    Dim sACNByteFound As Boolean = False

    Dim TimerSW As New Stopwatch
    Dim TimerPercent As Decimal = 0.0
    Dim T2Interval As Decimal = 0.0
    Dim ChPrev As New List(Of Integer)
    Dim si As Integer = 0
    Dim FormUpdateDivisor As Integer = 4
    Dim FormUpdateCount As Integer = 0

    Dim mycomputername As String = Environment.MachineName
    Dim mycomputerIP As New List(Of IPAddress)
    Dim Startup As Boolean = True
    Dim sACNConnected As Boolean = False

    Dim LightAddressOffset As Integer = 0
    Dim LH1OS As Integer = 8 'Actual channel... why did I do this?
    Dim LS1OS As Integer = LH1OS + 1
    Dim LV1OS As Integer = LS1OS + 1
    Dim LM1OS As Integer = LV1OS + 1
    Dim LH2OS As Integer = LM1OS + 1
    Dim LS2OS As Integer = LH2OS + 1
    Dim LV2OS As Integer = LS2OS + 1
    Dim LM2OS As Integer = LV2OS + 1
    Dim LH3OS As Integer = LM2OS + 1
    Dim LS3OS As Integer = LH3OS + 1
    Dim LV3OS As Integer = LS3OS + 1
    Dim LM3OS As Integer = LV3OS + 1
    Dim LH4OS As Integer = LM3OS + 1
    Dim LS4OS As Integer = LH4OS + 1
    Dim LV4OS As Integer = LS4OS + 1
    Dim LM4OS As Integer = LV4OS + 1
    Dim LH5OS As Integer = LM4OS + 1
    Dim LS5OS As Integer = LH5OS + 1
    Dim LV5OS As Integer = LS5OS + 1
    Dim LM5OS As Integer = LV5OS + 1
    Dim LH6OS As Integer = LM5OS + 1
    Dim LS6OS As Integer = LH6OS + 1
    Dim LV6OS As Integer = LS6OS + 1
    Dim LM6OS As Integer = LV6OS + 1
    Dim LH7OS As Integer = LM6OS + 1
    Dim LS7OS As Integer = LH7OS + 1
    Dim LV7OS As Integer = LS7OS + 1
    Dim LM7OS As Integer = LV7OS + 1
    Dim LH8OS As Integer = LM7OS + 1
    Dim LS8OS As Integer = LH8OS + 1
    Dim LV8OS As Integer = LS8OS + 1
    Dim LM8OS As Integer = LV8OS + 1
    Dim LH9OS As Integer = LM8OS + 1
    Dim LS9OS As Integer = LH9OS + 1
    Dim LV9OS As Integer = LS9OS + 1
    Dim LM9OS As Integer = LV9OS + 1
    Dim LH10OS As Integer = LM9OS + 1
    Dim LS10OS As Integer = LH10OS + 1
    Dim LV10OS As Integer = LS10OS + 1
    Dim LM10OS As Integer = LV10OS + 1
    Dim LH11OS As Integer = LM10OS + 1
    Dim LS11OS As Integer = LH11OS + 1
    Dim LV11OS As Integer = LS11OS + 1
    Dim LM11OS As Integer = LV11OS + 1
    Dim LH12OS As Integer = LM11OS + 1
    Dim LS12OS As Integer = LH12OS + 1
    Dim LV12OS As Integer = LS12OS + 1
    Dim LM12OS As Integer = LV12OS + 1

    Dim Pan1, Pan2, Tilt1, Tilt2, Disc1, Disc2, Disc3, PTSpeed, FXSel, FXSpeed As Integer

    'Form Load
    Private Sub Form1_Load(sender As Object, e As EventArgs) Handles MyBase.Load

        Startup = True

        Timer1.Interval = 30

        For Each Me.port In SerialList
            CBPort.Items.Add(port)
        Next port

        For i = 0 To channels - 1
            channelValues.Add(0)
            channelAddresses.Add(i + 1)
            ChPrev.Add(0)
        Next

        NMUDCh1Add.Value = 1
        NMUDCh2Add.Value = 2
        NMUDCh3Add.Value = 3
        NMUDCh4Add.Value = 4
        NMUDCh5Add.Value = 5
        NMUDCh6Add.Value = 6
        NMUDCh7Add.Value = 7
        NMUDCh8Add.Value = 8



        NetInt()
        Startup = False

    End Sub

    'Get Network Interfaces
    Public Sub NetInt()

        Dim mycomputerconnections() As System.Net.NetworkInformation.NetworkInterface = System.Net.NetworkInformation.NetworkInterface.GetAllNetworkInterfaces

        mycomputerIP.Clear()
        ComboBox1.Items.Clear()

        Dim IPz As String = "0.0.0.0"
        For i = 0 To mycomputerconnections.Length - 1
            IPz = "0.0.0.0"
            For i2 = 0 To mycomputerconnections(i).GetIPProperties.UnicastAddresses.Count - 1
                If mycomputerconnections(i).GetIPProperties.UnicastAddresses(i2).IsDnsEligible And (mycomputerconnections(i).GetIPProperties.UnicastAddresses(i2).PrefixLength = 24) Then
                    IPz = mycomputerconnections(i).GetIPProperties.UnicastAddresses(i2).Address.ToString
                    Exit For
                End If
            Next
            ComboBox1.Items.Add(mycomputerconnections(i).Name & " - " & IPz)
            mycomputerIP.Add(IPAddress.Parse(IPz))
            'If mycomputerconnections(i).Name.Contains("Ethernet") Then
            '    ComboBox1.SelectedIndex = i
            'End If
        Next
    End Sub

    'Connect
    Private Sub Button2_Click(sender As Object, e As EventArgs) Handles Button2.Click
        If SerialPort1.IsOpen = True Then
            SerialPort1.Close()
        End If
        Try
            SerialPort1.PortName = CBPort.Text
            SerialPort1.Open()
            SerialPort1.DtrEnable = True

            Timer1.Enabled = True
        Catch ex As Exception
            MsgBox("Either wrong port, or restart camera")
        End Try
    End Sub

    'Refresh
    Private Sub Button1_Click(sender As Object, e As EventArgs) Handles Button1.Click
        SerialList = System.IO.Ports.SerialPort.GetPortNames()
        CBPort.Items.Clear()
        For Each Me.port In SerialList
            CBPort.Items.Add(port)
        Next port
    End Sub

    'Ch1 Controls
    Private Sub NMUDCh1Value_ValueChanged(sender As Object, e As EventArgs) Handles NMUDCh1Value.ValueChanged
        If TBChanging = False Then
            NMUDchanging = True
            channelValues(NMUDCh1Add.Value - 1) = NMUDCh1Value.Value
            TBCh1.Value = NMUDCh1Value.Value
            NMUDchanging = False
        End If
    End Sub
    Private Sub TBCh1_Scroll(sender As Object, e As EventArgs) Handles TBCh1.Scroll
        If NMUDchanging = False Then
            TBChanging = True
            channelValues(NMUDCh1Add.Value - 1) = TBCh1.Value
            NMUDCh1Value.Value = TBCh1.Value
            TBChanging = False
        End If
    End Sub

    'Ch2 Controls
    Private Sub NMUDCh2Value_ValueChanged(sender As Object, e As EventArgs) Handles NMUDCh2Value.ValueChanged
        If TBChanging = False Then
            NMUDchanging = True
            channelValues(NMUDCh2Add.Value - 1) = NMUDCh2Value.Value
            TBCh2.Value = NMUDCh2Value.Value
            NMUDchanging = False
        End If
    End Sub
    Private Sub TBCh2_Scroll(sender As Object, e As EventArgs) Handles TBCh2.Scroll
        If NMUDchanging = False Then
            TBChanging = True
            channelValues(NMUDCh2Add.Value - 1) = TBCh2.Value
            NMUDCh2Value.Value = TBCh2.Value
            TBChanging = False
        End If
    End Sub

    'Ch3 Controls
    Private Sub NMUDCh3Value_ValueChanged(sender As Object, e As EventArgs) Handles NMUDCh3Value.ValueChanged
        If TBChanging = False Then
            NMUDchanging = True
            channelValues(NMUDCh3Add.Value - 1) = NMUDCh3Value.Value
            TBCh3.Value = NMUDCh3Value.Value
            NMUDchanging = False
        End If
    End Sub
    Private Sub TBCh3_Scroll(sender As Object, e As EventArgs) Handles TBCh3.Scroll
        If NMUDchanging = False Then
            TBChanging = True
            channelValues(NMUDCh3Add.Value - 1) = TBCh3.Value
            NMUDCh3Value.Value = TBCh3.Value
            TBChanging = False
        End If
    End Sub

    'Ch4 Controls
    Private Sub NMUDCh4Value_ValueChanged(sender As Object, e As EventArgs) Handles NMUDCh4Value.ValueChanged
        If TBChanging = False Then
            NMUDchanging = True
            channelValues(NMUDCh4Add.Value - 1) = NMUDCh4Value.Value
            TBCh4.Value = NMUDCh4Value.Value
            NMUDchanging = False
        End If
    End Sub
    Private Sub TBCh4_Scroll(sender As Object, e As EventArgs) Handles TBCh4.Scroll
        If NMUDchanging = False Then
            TBChanging = True
            channelValues(NMUDCh4Add.Value - 1) = TBCh4.Value
            NMUDCh4Value.Value = TBCh4.Value
            TBChanging = False
        End If
    End Sub

    'Ch5 Controls
    Private Sub NMUDCh5Value_ValueChanged(sender As Object, e As EventArgs) Handles NMUDCh5Value.ValueChanged
        If TBChanging = False Then
            NMUDchanging = True
            channelValues(NMUDCh5Add.Value - 1) = NMUDCh5Value.Value
            TBCh5.Value = NMUDCh5Value.Value
            NMUDchanging = False
        End If
    End Sub
    Private Sub TBCh5_Scroll(sender As Object, e As EventArgs) Handles TBCh5.Scroll
        If NMUDchanging = False Then
            TBChanging = True
            channelValues(NMUDCh5Add.Value - 1) = TBCh5.Value
            NMUDCh5Value.Value = TBCh5.Value
            TBChanging = False
        End If
    End Sub

    'Ch6 Controls
    Private Sub NMUDCh6Value_ValueChanged(sender As Object, e As EventArgs) Handles NMUDCh6Value.ValueChanged
        If TBChanging = False Then
            NMUDchanging = True
            channelValues(NMUDCh6Add.Value - 1) = NMUDCh6Value.Value
            TBCh6.Value = NMUDCh6Value.Value
            NMUDchanging = False
        End If
    End Sub
    Private Sub TBCh6_Scroll(sender As Object, e As EventArgs) Handles TBCh6.Scroll
        If NMUDchanging = False Then
            TBChanging = True
            channelValues(NMUDCh6Add.Value - 1) = TBCh6.Value
            NMUDCh6Value.Value = TBCh6.Value
            TBChanging = False
        End If
    End Sub

    'Ch7 Controls
    Private Sub NMUDCh7Value_ValueChanged(sender As Object, e As EventArgs) Handles NMUDCh7Value.ValueChanged
        If TBChanging = False Then
            NMUDchanging = True
            channelValues(NMUDCh7Add.Value - 1) = NMUDCh7Value.Value
            TBCh7.Value = NMUDCh7Value.Value
            NMUDchanging = False
        End If
    End Sub
    Private Sub TBCh7_Scroll(sender As Object, e As EventArgs) Handles TBCh7.Scroll
        If NMUDchanging = False Then
            TBChanging = True
            channelValues(NMUDCh7Add.Value - 1) = TBCh7.Value
            NMUDCh7Value.Value = TBCh7.Value
            TBChanging = False
        End If
    End Sub

    'Ch8 Controls
    Private Sub NMUDCh8Value_ValueChanged(sender As Object, e As EventArgs) Handles NMUDCh8Value.ValueChanged
        If TBChanging = False Then
            NMUDchanging = True
            channelValues(NMUDCh8Add.Value - 1) = NMUDCh8Value.Value
            TBCh8.Value = NMUDCh8Value.Value
            NMUDchanging = False
        End If
    End Sub
    Private Sub TBCh8_Scroll(sender As Object, e As EventArgs) Handles TBCh8.Scroll
        If NMUDchanging = False Then
            TBChanging = True
            channelValues(NMUDCh8Add.Value - 1) = TBCh8.Value
            NMUDCh8Value.Value = TBCh8.Value
            TBChanging = False
        End If
    End Sub

    'DMX Send
    Private Sub Timer1_Tick(sender As Object, e As EventArgs) Handles Timer1.Tick

        Dim sendlist As New List(Of Byte)

        channelValues(0) = Tilt1
        channelValues(1) = Tilt2
        channelValues(2) = Pan1
        channelValues(3) = Pan2
        channelValues(4) = Disc1
        channelValues(5) = Disc2
        channelValues(6) = Disc3
        'channelValues(55) = PTSpeed
        'channelValues(56) = FXSel
        'channelValues(57) = FXSpeed

        For i = 0 To channels - 1
            sendlist.Add(channelValues(i))
        Next
        Dim sendstring As Byte() = sendlist.ToArray()

        Try
            SerialPort1.Write(sendstring, 0, 70)

        Catch ex As Exception
            Timer1.Enabled = False

            MsgBox("Serial Unplugged, reconnect")
        End Try
    End Sub

    'Add
    Private Sub Button3_Click(sender As Object, e As EventArgs) Handles Button3.Click

        sceneNames.Add(TextBox1.Text)
        SceneChannels.Add(New List(Of Integer))
        sceneValues.Add(New List(Of Integer))
        Dim si As Integer = SceneChannels.Count - 1
        SceneChannels(si).Add(NMUDCh1Add.Value)
        sceneValues(si).Add(NMUDCh1Value.Value)
        SceneChannels(si).Add(NMUDCh2Add.Value)
        sceneValues(si).Add(NMUDCh2Value.Value)
        SceneChannels(si).Add(NMUDCh3Add.Value)
        sceneValues(si).Add(NMUDCh3Value.Value)
        SceneChannels(si).Add(NMUDCh4Add.Value)
        sceneValues(si).Add(NMUDCh4Value.Value)
        SceneChannels(si).Add(NMUDCh5Add.Value)
        sceneValues(si).Add(NMUDCh5Value.Value)
        SceneChannels(si).Add(NMUDCh6Add.Value)
        sceneValues(si).Add(NMUDCh6Value.Value)
        SceneChannels(si).Add(NMUDCh7Add.Value)
        sceneValues(si).Add(NMUDCh7Value.Value)
        SceneChannels(si).Add(NMUDCh8Add.Value)
        sceneValues(si).Add(NMUDCh8Value.Value)

        UpdateSceneListbox()

    End Sub

    'Del
    Private Sub Button4_Click(sender As Object, e As EventArgs) Handles Button4.Click
        If ListBox1.SelectedIndex = -1 Then
            Return
        End If

        sceneNames.RemoveAt(ListBox1.SelectedIndex)
        sceneValues.RemoveAt(ListBox1.SelectedIndex)
        SceneChannels.RemoveAt(ListBox1.SelectedIndex)

        If sceneNames.Count = 0 Then
            Timer2.Enabled = False
        End If

        UpdateSceneListbox()

    End Sub

    'Scenes List box update
    Public Sub UpdateSceneListbox()
        ListBox1.Items.Clear()
        For i = 0 To sceneNames.Count - 1
            ListBox1.Items.Add((i + 1) & " - " & sceneNames(i))
        Next
    End Sub

    'Up
    Private Sub Button5_Click(sender As Object, e As EventArgs) Handles Button5.Click
        If ListBox1.SelectedIndex = -1 Or ListBox1.SelectedIndex = 0 Then
            Return
        End If

        Dim si = ListBox1.SelectedIndex

        Dim tempstring As String = sceneNames(si)
        Dim tempchannels As List(Of Integer) = SceneChannels(si)
        Dim tempvalues As List(Of Integer) = sceneValues(si)

        sceneNames.RemoveAt(si)
        sceneValues.RemoveAt(si)
        SceneChannels.RemoveAt(si)

        sceneNames.Insert(si - 1, tempstring)
        SceneChannels.Insert(si - 1, tempchannels)
        sceneValues.Insert(si - 1, tempvalues)

        UpdateSceneListbox()

    End Sub

    'Down
    Private Sub Button6_Click(sender As Object, e As EventArgs) Handles Button6.Click
        If ListBox1.SelectedIndex = -1 Or ListBox1.SelectedIndex = sceneNames.Count - 1 Then
            Return
        End If

        Dim si = ListBox1.SelectedIndex

        Dim tempstring As String = sceneNames(si)
        Dim tempchannels As List(Of Integer) = SceneChannels(si)
        Dim tempvalues As List(Of Integer) = sceneValues(si)

        sceneNames.RemoveAt(si)
        sceneValues.RemoveAt(si)
        SceneChannels.RemoveAt(si)

        sceneNames.Insert(si + 1, tempstring)
        SceneChannels.Insert(si + 1, tempchannels)
        sceneValues.Insert(si + 1, tempvalues)

        UpdateSceneListbox()

    End Sub

    'Apply
    Private Sub Button7_Click(sender As Object, e As EventArgs) Handles Button7.Click
        If ListBox1.SelectedIndex = -1 Then
            Return
        End If


        Dim si As Integer = ListBox1.SelectedIndex
        sceneNames(si) = TextBox1.Text
        SceneChannels(si)(0) = (NMUDCh1Add.Value)
        sceneValues(si)(0) = (NMUDCh1Value.Value)
        SceneChannels(si)(1) = (NMUDCh2Add.Value)
        sceneValues(si)(1) = (NMUDCh2Value.Value)
        SceneChannels(si)(2) = (NMUDCh3Add.Value)
        sceneValues(si)(2) = (NMUDCh3Value.Value)
        SceneChannels(si)(3) = (NMUDCh4Add.Value)
        sceneValues(si)(3) = (NMUDCh4Value.Value)
        SceneChannels(si)(4) = (NMUDCh5Add.Value)
        sceneValues(si)(4) = (NMUDCh5Value.Value)
        SceneChannels(si)(5) = (NMUDCh6Add.Value)
        sceneValues(si)(5) = (NMUDCh6Value.Value)
        SceneChannels(si)(6) = (NMUDCh7Add.Value)
        sceneValues(si)(6) = (NMUDCh7Value.Value)
        SceneChannels(si)(7) = (NMUDCh8Add.Value)
        sceneValues(si)(7) = (NMUDCh8Value.Value)

        UpdateSceneListbox()

    End Sub

    Private Sub ListBox1_SelectedIndexChanged(sender As Object, e As EventArgs) Handles ListBox1.SelectedIndexChanged
        If ListBox1.SelectedIndex = -1 Then
            Return
        End If

        si = ListBox1.SelectedIndex
        TextBox1.Text = sceneNames(si)
        NMUDCh1Add.Value = SceneChannels(si)(0)
        NMUDCh2Add.Value = SceneChannels(si)(1)
        NMUDCh3Add.Value = SceneChannels(si)(2)
        NMUDCh4Add.Value = SceneChannels(si)(3)
        NMUDCh5Add.Value = SceneChannels(si)(4)
        NMUDCh6Add.Value = SceneChannels(si)(5)
        NMUDCh7Add.Value = SceneChannels(si)(6)
        NMUDCh8Add.Value = SceneChannels(si)(7)


        If Timer2.Enabled = False Then

            NMUDchanging = True
            TBCh1.Value = sceneValues(si)(0)
            NMUDCh1Value.Value = sceneValues(si)(0)
            TBCh2.Value = sceneValues(si)(1)
            NMUDCh2Value.Value = sceneValues(si)(1)
            TBCh3.Value = sceneValues(si)(2)
            NMUDCh3Value.Value = sceneValues(si)(2)
            TBCh4.Value = sceneValues(si)(3)
            NMUDCh4Value.Value = sceneValues(si)(3)
            TBCh5.Value = sceneValues(si)(4)
            NMUDCh5Value.Value = sceneValues(si)(4)
            TBCh6.Value = sceneValues(si)(5)
            NMUDCh6Value.Value = sceneValues(si)(5)
            TBCh7.Value = sceneValues(si)(6)
            NMUDCh7Value.Value = sceneValues(si)(6)
            TBCh8.Value = sceneValues(si)(7)
            NMUDCh8Value.Value = sceneValues(si)(7)
            NMUDchanging = False
        End If

    End Sub

    'Sequencer Tick
    Private Sub Timer2_Tick(sender As Object, e As EventArgs) Handles Timer2.Tick

        TimerSW.Stop()
        TimerSW.Reset()
        TimerSW.Start()
        For i = 0 To ChPrev.Count - 1
            ChPrev(i) = channelValues(i)
        Next

        If ListBox1.SelectedIndex < ListBox1.Items.Count - 1 Then
            ListBox1.SelectedIndex = ListBox1.SelectedIndex + 1
        Else
            ListBox1.SelectedIndex = 0
        End If

    End Sub

    'Start
    Private Sub Button8_Click(sender As Object, e As EventArgs) Handles Button8.Click
        If sceneNames.Count > 0 Then
            Timer2.Interval = NumericUpDown1.Value
            Timer2.Enabled = True
            TimerSW.Stop()
            TimerSW.Reset()
            TimerSW.Start()
            For i = 0 To ChPrev.Count - 1
                ChPrev(i) = channelValues(i)
            Next
        End If
        If sACNActive = True Then
            Timer3.Enabled = True
            TimerSW.Reset()
            TimerSW.Start()
        End If
    End Sub

    'Stop
    Private Sub Button9_Click(sender As Object, e As EventArgs) Handles Button9.Click
        Timer2.Enabled = False
        'Timer3.Enabled = False
    End Sub

    'UpdateTimer
    Private Sub NumericUpDown1_ValueChanged(sender As Object, e As EventArgs) Handles NumericUpDown1.ValueChanged
        Timer2.Interval = NumericUpDown1.Value
        T2Interval = NumericUpDown1.Value
    End Sub

    'Select DMX
    Private Sub RadioButton1_CheckedChanged(sender As Object, e As EventArgs) Handles RadioButton1.CheckedChanged
        If RadioButton1.Checked = True Then
            Button1.Enabled = True
            Button2.Enabled = True
            CBPort.Enabled = True
            Label1.Enabled = True
            Button10.Enabled = False
            Button11.Enabled = False
            ComboBox1.Enabled = False
            Button12.Enabled = False
        End If

    End Sub
    'Select sACN
    Private Sub RadioButton2_CheckedChanged(sender As Object, e As EventArgs) Handles RadioButton2.CheckedChanged
        If RadioButton2.Checked = True Then
            Button1.Enabled = False
            Button2.Enabled = False
            CBPort.Enabled = False
            Label1.Enabled = False
            ComboBox1.Enabled = True
            ComboBox1.DroppedDown = True
            Button12.Enabled = True
        End If
    End Sub

    Private Sub sACN()


        Dim bytes As New List(Of Byte)
        bytes.Add(0)    'Preamble Size
        bytes.Add(16)   'Preamble Size
        bytes.Add(0)    'Postamble Size
        bytes.Add(0)    'Postamble Size
        bytes.Add(65)   'Packet ID
        bytes.Add(83)
        bytes.Add(67)
        bytes.Add(45)
        bytes.Add(69)
        bytes.Add(49)
        bytes.Add(46)
        bytes.Add(49)
        bytes.Add(55)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(114)  'Flags / Length
        bytes.Add(110)  'Length
        bytes.Add(0)    'Protocol
        bytes.Add(0)    'Protocol
        bytes.Add(0)    'Protocol
        bytes.Add(4)    'Protocol
        bytes.Add(170)  'CID
        bytes.Add(23)
        bytes.Add(64)
        bytes.Add(177)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(7)
        bytes.Add(127)
        bytes.Add(156)
        bytes.Add(67)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(114)  'Flags / Length
        bytes.Add(88)   'Length
        bytes.Add(0)    'Vector
        bytes.Add(0)    'Vector
        bytes.Add(0)    'Vector
        bytes.Add(2)    'Vector
        bytes.Add(87)   'Source Name
        bytes.Add(67)
        bytes.Add(32)
        bytes.Add(74)
        bytes.Add(97)
        bytes.Add(110)
        bytes.Add(100)
        bytes.Add(115)
        bytes.Add(32)
        bytes.Add(86)
        bytes.Add(105)
        bytes.Add(115)
        bytes.Add(116)
        bytes.Add(97)
        bytes.Add(32)
        bytes.Add(76)
        bytes.Add(53)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(0)
        bytes.Add(100)  'Priority
        bytes.Add(0)    'Sync Universe
        bytes.Add(0)    'Sync Universe
        bytes.Add(4)    'Seq Number
        bytes.Add(0)    'Options
        bytes.Add(0)    'Universe
        bytes.Add(1)    'Universe
        bytes.Add(114)  'Flags / Length
        bytes.Add(11)   'Length
        bytes.Add(2)    'DNP Vector
        bytes.Add(161)  'Other Misc Flags
        bytes.Add(0)    'First Property Address
        bytes.Add(0)    'First Property Address
        bytes.Add(0)    'Increment
        bytes.Add(1)    'Increment
        bytes.Add(2)    'Count
        bytes.Add(1)    'Count
        bytes.Add(0)    'Start Code
        For i = 0 To 511
            bytes.Add(channelValues(i))
        Next

        Dim bytez = bytes.ToArray
        sACNUDP.Send(bytez, bytez.Length, sACNEP)

    End Sub

    'Linear percentage
    Private Function LP(ByVal A As Integer, ByVal B As Integer, ByVal P As Decimal)
        Return A + (B - A) * P
    End Function
    'sACN Start
    Private Sub Button10_Click(sender As Object, e As EventArgs) Handles Button10.Click
        Timer3.Enabled = True
        sACNActive = True
    End Sub
    'sACN Stop
    Private Sub Button11_Click(sender As Object, e As EventArgs) Handles Button11.Click
        Timer3.Enabled = False
        sACNActive = False
    End Sub
    'sACN Tick
    Private Sub Timer3_Tick(sender As Object, e As EventArgs) Handles Timer3.Tick


        If Timer2.Enabled = True Then
            TimerPercent = Math.Min((TimerSW.ElapsedMilliseconds / T2Interval), 1)
        Else
            TimerPercent = 1
        End If

        If ListBox1.SelectedIndex <> -1 And Timer2.Enabled = True Then
            channelAddresses(0) = SceneChannels(si)(0)
            channelValues(0) = LP(ChPrev(0), sceneValues(si)(0), TimerPercent)
            channelAddresses(1) = SceneChannels(si)(1)
            channelValues(1) = LP(ChPrev(1), sceneValues(si)(1), TimerPercent)
            channelAddresses(2) = SceneChannels(si)(2)
            channelValues(2) = LP(ChPrev(2), sceneValues(si)(2), TimerPercent)
            channelAddresses(3) = SceneChannels(si)(3)
            channelValues(3) = LP(ChPrev(3), sceneValues(si)(3), TimerPercent)
            channelAddresses(4) = SceneChannels(si)(4)
            channelValues(4) = LP(ChPrev(4), sceneValues(si)(4), TimerPercent)
            channelAddresses(5) = SceneChannels(si)(5)
            channelValues(5) = LP(ChPrev(5), sceneValues(si)(5), TimerPercent)
            channelAddresses(6) = SceneChannels(si)(6)
            channelValues(6) = LP(ChPrev(6), sceneValues(si)(6), TimerPercent)
            channelAddresses(7) = SceneChannels(si)(7)
            channelValues(7) = LP(ChPrev(7), sceneValues(si)(7), TimerPercent)

            FormUpdateCount = FormUpdateCount + 1
            If FormUpdateCount >= FormUpdateDivisor Then
                NMUDchanging = True
                TBChanging = True
                NMUDCh1Value.Value = channelValues(0)
                NMUDCh2Value.Value = channelValues(1)
                NMUDCh3Value.Value = channelValues(2)
                NMUDCh4Value.Value = channelValues(3)
                NMUDCh5Value.Value = channelValues(4)
                NMUDCh6Value.Value = channelValues(5)
                NMUDCh7Value.Value = channelValues(6)
                NMUDCh8Value.Value = channelValues(7)
                TBCh1.Value = channelValues(0)
                TBCh2.Value = channelValues(1)
                TBCh3.Value = channelValues(2)
                TBCh4.Value = channelValues(3)
                TBCh5.Value = channelValues(4)
                TBCh6.Value = channelValues(5)
                TBCh7.Value = channelValues(6)
                TBCh8.Value = channelValues(7)
                NMUDchanging = False
                TBChanging = False
                FormUpdateCount = 0
            End If

        End If


        sACN()
    End Sub

    'Select Ethernet Connection
    Private Sub ComboBox1_SelectedIndexChanged(sender As Object, e As EventArgs) Handles ComboBox1.SelectedIndexChanged
        If Startup = False And ComboBox1.SelectedIndex <> -1 Then
            If sACNConnected = True Then
                sACNUDP.DropMulticastGroup(sACNIP)
                sACNUDP.DropMulticastGroup(sACNIP2)
                sACNConnected = False
            End If

            Try
                localIP = mycomputerIP(ComboBox1.SelectedIndex)
                sACNUDP.Client.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, True)
                sACNUDP.JoinMulticastGroup(sACNIP, localIP)
                sACNUDP.JoinMulticastGroup(sACNIP2, localIP)
                sACNUDP.Client.Ttl = 5
                sACNConnected = True
                Button10.Enabled = True
                Button11.Enabled = True
            Catch ex As Exception
                MsgBox("Not a good connection")
            End Try


        End If
    End Sub

    'Network Refresh
    Private Sub Button12_Click(sender As Object, e As EventArgs) Handles Button12.Click
        NetInt()
    End Sub

    'Channel Changes
    Private Sub NMUDCh1Add_ValueChanged(sender As Object, e As EventArgs) Handles NMUDCh1Add.ValueChanged
        If Startup = False Then
            NMUDchanging = True
            TBChanging = True
            NMUDCh1Value.Value = channelValues(NMUDCh1Add.Value - 1)
            TBCh1.Value = channelValues(NMUDCh1Add.Value - 1)
            NMUDchanging = False
            TBChanging = False
        End If
    End Sub
    Private Sub NMUDCh2Add_ValueChanged(sender As Object, e As EventArgs) Handles NMUDCh2Add.ValueChanged
        If Startup = False Then
            NMUDchanging = True
            TBChanging = True
            NMUDCh2Value.Value = channelValues(NMUDCh2Add.Value - 1)
            TBCh2.Value = channelValues(NMUDCh2Add.Value - 1)
            NMUDchanging = False
            TBChanging = False
        End If
    End Sub
    Private Sub NMUDCh3Add_ValueChanged(sender As Object, e As EventArgs) Handles NMUDCh3Add.ValueChanged
        If Startup = False Then
            NMUDchanging = True
            TBChanging = True
            NMUDCh3Value.Value = channelValues(NMUDCh3Add.Value - 1)
            TBCh3.Value = channelValues(NMUDCh3Add.Value - 1)
            NMUDchanging = False
            TBChanging = False
        End If
    End Sub
    Private Sub NMUDCh4Add_ValueChanged(sender As Object, e As EventArgs) Handles NMUDCh4Add.ValueChanged
        If Startup = False Then
            NMUDchanging = True
            TBChanging = True
            NMUDCh4Value.Value = channelValues(NMUDCh4Add.Value - 1)
            TBCh4.Value = channelValues(NMUDCh4Add.Value - 1)
            NMUDchanging = False
            TBChanging = False
        End If
    End Sub
    Private Sub NMUDCh5Add_ValueChanged(sender As Object, e As EventArgs) Handles NMUDCh5Add.ValueChanged
        If Startup = False Then
            NMUDchanging = True
            TBChanging = True
            NMUDCh5Value.Value = channelValues(NMUDCh5Add.Value - 1)
            TBCh5.Value = channelValues(NMUDCh5Add.Value - 1)
            NMUDchanging = False
            TBChanging = False
        End If
    End Sub

    Private Sub NMUDCh6Add_ValueChanged(sender As Object, e As EventArgs) Handles NMUDCh6Add.ValueChanged
        If Startup = False Then
            NMUDchanging = True
            TBChanging = True
            NMUDCh6Value.Value = channelValues(NMUDCh6Add.Value - 1)
            TBCh6.Value = channelValues(NMUDCh6Add.Value - 1)
            NMUDchanging = False
            TBChanging = False
        End If
    End Sub

    Private Sub NMUDCh7Add_ValueChanged(sender As Object, e As EventArgs) Handles NMUDCh7Add.ValueChanged
        If Startup = False Then
            NMUDchanging = True
            TBChanging = True
            NMUDCh7Value.Value = channelValues(NMUDCh7Add.Value - 1)
            TBCh7.Value = channelValues(NMUDCh7Add.Value - 1)
            NMUDchanging = False
            TBChanging = False
        End If
    End Sub
    Private Sub NMUDCh8Add_ValueChanged(sender As Object, e As EventArgs) Handles NMUDCh8Add.ValueChanged
        If Startup = False Then
            NMUDchanging = True
            TBChanging = True
            NMUDCh8Value.Value = channelValues(NMUDCh8Add.Value - 1)
            TBCh8.Value = channelValues(NMUDCh8Add.Value - 1)
            NMUDchanging = False
            TBChanging = False
        End If
    End Sub

    'Picture Trackbars
    Private Sub TBPicMirror_Scroll(sender As Object, e As EventArgs) Handles TBPicMirror.Scroll
        If CB1.Checked = True Then
            channelValues(LightAddressOffset + LM1OS - 1) = TBPicMirror.Value
        End If
        If CB2.Checked = True Then
            channelValues(LightAddressOffset + LM2OS - 1) = TBPicMirror.Value
        End If
        If CB3.Checked = True Then
            channelValues(LightAddressOffset + LM3OS - 1) = TBPicMirror.Value
        End If
        If CB4.Checked = True Then
            channelValues(LightAddressOffset + LM4OS - 1) = TBPicMirror.Value
        End If
        If CB5.Checked = True Then
            channelValues(LightAddressOffset + LM5OS - 1) = TBPicMirror.Value
        End If
        If CB6.Checked = True Then
            channelValues(LightAddressOffset + LM6OS - 1) = TBPicMirror.Value
        End If
        If CB7.Checked = True Then
            channelValues(LightAddressOffset + LM7OS - 1) = TBPicMirror.Value
        End If
        If CB8.Checked = True Then
            channelValues(LightAddressOffset + LM8OS - 1) = TBPicMirror.Value
        End If
        If CB9.Checked = True Then
            channelValues(LightAddressOffset + LM9OS - 1) = TBPicMirror.Value
        End If
        If CB10.Checked = True Then
            channelValues(LightAddressOffset + LM10OS - 1) = TBPicMirror.Value
        End If
        If CB11.Checked = True Then
            channelValues(LightAddressOffset + LM11OS - 1) = TBPicMirror.Value
        End If
        If CB12.Checked = True Then
            channelValues(LightAddressOffset + LM12OS - 1) = TBPicMirror.Value
        End If
    End Sub

    Private Sub TBPicHue_Scroll(sender As Object, e As EventArgs) Handles TBPicHue.Scroll
        If CB1.Checked = True Then
            channelValues(LightAddressOffset + LH1OS - 1) = TBPicHue.Value
        End If
        If CB2.Checked = True Then
            channelValues(LightAddressOffset + LH2OS - 1) = TBPicHue.Value
        End If
        If CB3.Checked = True Then
            channelValues(LightAddressOffset + LH3OS - 1) = TBPicHue.Value
        End If
        If CB4.Checked = True Then
            channelValues(LightAddressOffset + LH4OS - 1) = TBPicHue.Value
        End If
        If CB5.Checked = True Then
            channelValues(LightAddressOffset + LH5OS - 1) = TBPicHue.Value
        End If
        If CB6.Checked = True Then
            channelValues(LightAddressOffset + LH6OS - 1) = TBPicHue.Value
        End If
        If CB7.Checked = True Then
            channelValues(LightAddressOffset + LH7OS - 1) = TBPicHue.Value
        End If
        If CB8.Checked = True Then
            channelValues(LightAddressOffset + LH8OS - 1) = TBPicHue.Value
        End If
        If CB9.Checked = True Then
            channelValues(LightAddressOffset + LH9OS - 1) = TBPicHue.Value
        End If
        If CB10.Checked = True Then
            channelValues(LightAddressOffset + LH10OS - 1) = TBPicHue.Value
        End If
        If CB11.Checked = True Then
            channelValues(LightAddressOffset + LH11OS - 1) = TBPicHue.Value
        End If
        If CB12.Checked = True Then
            channelValues(LightAddressOffset + LH12OS - 1) = TBPicHue.Value
        End If
    End Sub

    Private Sub TBPicSat_Scroll(sender As Object, e As EventArgs) Handles TBPicSat.Scroll
        If CB1.Checked = True Then
            channelValues(LightAddressOffset + LS1OS - 1) = TBPicSat.Value
        End If
        If CB2.Checked = True Then
            channelValues(LightAddressOffset + LS2OS - 1) = TBPicSat.Value
        End If
        If CB3.Checked = True Then
            channelValues(LightAddressOffset + LS3OS - 1) = TBPicSat.Value
        End If
        If CB4.Checked = True Then
            channelValues(LightAddressOffset + LS4OS - 1) = TBPicSat.Value
        End If
        If CB5.Checked = True Then
            channelValues(LightAddressOffset + LS5OS - 1) = TBPicSat.Value
        End If
        If CB6.Checked = True Then
            channelValues(LightAddressOffset + LS6OS - 1) = TBPicSat.Value
        End If
        If CB7.Checked = True Then
            channelValues(LightAddressOffset + LS7OS - 1) = TBPicSat.Value
        End If
        If CB8.Checked = True Then
            channelValues(LightAddressOffset + LS8OS - 1) = TBPicSat.Value
        End If
        If CB9.Checked = True Then
            channelValues(LightAddressOffset + LS9OS - 1) = TBPicSat.Value
        End If
        If CB10.Checked = True Then
            channelValues(LightAddressOffset + LS10OS - 1) = TBPicSat.Value
        End If
        If CB11.Checked = True Then
            channelValues(LightAddressOffset + LS11OS - 1) = TBPicSat.Value
        End If
        If CB12.Checked = True Then
            channelValues(LightAddressOffset + LS12OS - 1) = TBPicSat.Value
        End If
    End Sub

    Private Sub TBPicValue_Scroll(sender As Object, e As EventArgs) Handles TBPicValue.Scroll
        If CB1.Checked = True Then
            channelValues(LightAddressOffset + LV1OS - 1) = TBPicValue.Value
        End If
        If CB2.Checked = True Then
            channelValues(LightAddressOffset + LV2OS - 1) = TBPicValue.Value
        End If
        If CB3.Checked = True Then
            channelValues(LightAddressOffset + LV3OS - 1) = TBPicValue.Value
        End If
        If CB4.Checked = True Then
            channelValues(LightAddressOffset + LV4OS - 1) = TBPicValue.Value
        End If
        If CB5.Checked = True Then
            channelValues(LightAddressOffset + LV5OS - 1) = TBPicValue.Value
        End If
        If CB6.Checked = True Then
            channelValues(LightAddressOffset + LV6OS - 1) = TBPicValue.Value
        End If
        If CB7.Checked = True Then
            channelValues(LightAddressOffset + LV7OS - 1) = TBPicValue.Value
        End If
        If CB8.Checked = True Then
            channelValues(LightAddressOffset + LV8OS - 1) = TBPicValue.Value
        End If
        If CB9.Checked = True Then
            channelValues(LightAddressOffset + LV9OS - 1) = TBPicValue.Value
        End If
        If CB10.Checked = True Then
            channelValues(LightAddressOffset + LV10OS - 1) = TBPicValue.Value
        End If
        If CB11.Checked = True Then
            channelValues(LightAddressOffset + LV11OS - 1) = TBPicValue.Value
        End If
        If CB12.Checked = True Then
            channelValues(LightAddressOffset + LV12OS - 1) = TBPicValue.Value
        End If
    End Sub

    Private Sub CBAllNone_CheckedChanged(sender As Object, e As EventArgs) Handles CBAllNone.CheckedChanged
        If CBAllNone.Checked = True Then
            CB1.Checked = True
            CB2.Checked = True
            CB3.Checked = True
            CB4.Checked = True
            CB5.Checked = True
            CB6.Checked = True
            CB7.Checked = True
            CB8.Checked = True
            CB9.Checked = True
            CB10.Checked = True
            CB11.Checked = True
            CB12.Checked = True
        Else
            CB1.Checked = False
            CB2.Checked = False
            CB3.Checked = False
            CB4.Checked = False
            CB5.Checked = False
            CB6.Checked = False
            CB7.Checked = False
            CB8.Checked = False
            CB9.Checked = False
            CB10.Checked = False
            CB11.Checked = False
            CB12.Checked = False
        End If
    End Sub

    Private Sub TBPicDiscSpeed_Scroll(sender As Object, e As EventArgs) Handles TBPicDiscSpeed.Scroll
        Disc1 = TBPicDiscSpeed.Value
    End Sub

    Private Sub TBPicDiscDir_Scroll(sender As Object, e As EventArgs) Handles TBPicDiscDir.Scroll
        Disc2 = TBPicDiscDir.Value
    End Sub

    Private Sub TBPicTilt_Scroll(sender As Object, e As EventArgs) Handles TBPicTilt.Scroll
        Tilt1 = TBPicTilt.Value
    End Sub

End Class

VERSION 5.00
Begin VB.Form frmDScalerAPITest 
   Caption         =   "DScaler API Test"
   ClientHeight    =   3195
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   4680
   LinkTopic       =   "Form1"
   ScaleHeight     =   3195
   ScaleWidth      =   4680
   StartUpPosition =   3  'Windows Default
   Begin VB.TextBox txtValueNum 
      Height          =   315
      Left            =   2250
      TabIndex        =   14
      Top             =   420
      Width           =   1950
   End
   Begin VB.CommandButton cmdSendMessage 
      Caption         =   "Send Message"
      Height          =   480
      Left            =   240
      TabIndex        =   10
      Top             =   2520
      Width           =   1650
   End
   Begin VB.ComboBox cmbChangeType 
      Height          =   315
      ItemData        =   "frmDScalerAPITest.frx":0000
      Left            =   2340
      List            =   "frmDScalerAPITest.frx":002B
      Style           =   2  'Dropdown List
      TabIndex        =   7
      Top             =   1890
      Width           =   2100
   End
   Begin VB.TextBox txtSetValue 
      Height          =   285
      Left            =   225
      TabIndex        =   5
      Top             =   1860
      Width           =   1695
   End
   Begin VB.Frame Frame1 
      Caption         =   "Message type"
      Height          =   585
      Left            =   210
      TabIndex        =   3
      Top             =   870
      Width           =   4245
      Begin VB.OptionButton Option1 
         Caption         =   "Change"
         Height          =   255
         Index           =   2
         Left            =   2625
         TabIndex        =   12
         Top             =   210
         Width           =   1455
      End
      Begin VB.OptionButton Option1 
         Caption         =   "Set"
         Height          =   255
         Index           =   1
         Left            =   1425
         TabIndex        =   11
         Top             =   210
         Width           =   1455
      End
      Begin VB.OptionButton Option1 
         Caption         =   "Get"
         Height          =   255
         Index           =   0
         Left            =   120
         TabIndex        =   4
         Top             =   210
         Value           =   -1  'True
         Width           =   1455
      End
   End
   Begin VB.TextBox txtResult 
      Enabled         =   0   'False
      Height          =   285
      Left            =   2220
      TabIndex        =   1
      Top             =   2760
      Width           =   1830
   End
   Begin VB.TextBox txtOffset 
      Height          =   300
      Left            =   210
      TabIndex        =   0
      Top             =   420
      Width           =   1860
   End
   Begin VB.Label Label5 
      Caption         =   "Value Number"
      Height          =   210
      Left            =   2250
      TabIndex        =   13
      Top             =   180
      Width           =   1110
   End
   Begin VB.Label Label4 
      Caption         =   "Result"
      Height          =   195
      Left            =   2250
      TabIndex        =   9
      Top             =   2505
      Width           =   1725
   End
   Begin VB.Label Label3 
      Caption         =   "Change Type"
      Height          =   180
      Left            =   2370
      TabIndex        =   8
      Top             =   1635
      Width           =   1275
   End
   Begin VB.Label Label2 
      Caption         =   "Set value"
      Height          =   225
      Left            =   225
      TabIndex        =   6
      Top             =   1620
      Width           =   1590
   End
   Begin VB.Label Label1 
      Caption         =   "Offset"
      Height          =   270
      Left            =   210
      TabIndex        =   2
      Top             =   150
      Width           =   1440
   End
End
Attribute VB_Name = "frmDScalerAPITest"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Declare Function FindWindow Lib "user32" Alias "FindWindowA" (ByVal lpClassName As String, ByVal lpWindowName As Long) As Long
Private Declare Function SendMessage Lib "user32" Alias "SendMessageA" (ByVal hWnd As Long, ByVal wMsg As Long, ByVal wParam As Long, ByVal lParam As Long) As Long
Private Const WM_APP As Long = 32768

Private Sub cmdSendMessage_Click()
Dim hWnd As Long
Dim Offset As Long
Dim Message As Long
Dim wParam As Long
Dim lParam As Long
Dim Result As Long

On Error Resume Next

hWnd = FindWindow("DScaler", 0)
If hWnd = 0 Then
    MsgBox "Can't find DScaler Window!, Are you sure it's running?", vbCritical, "Error"
    Exit Sub
End If

Offset = txtOffset

If Offset <= 0 Or Offset >= 100 Then
    MsgBox "Invalid offset", vbCritical, "Error"
    Exit Sub
End If

If Option1(0).Value = True Then
    Message = WM_APP + Offset
ElseIf Option1(1).Value = True Then
    Message = WM_APP + Offset + 100
Else
    Message = WM_APP + Offset + 200
End If

wParam = txtValueNum

If wParam < 0 Then
    MsgBox "Invalid Value Num", vbCritical, "Error"
    Exit Sub
End If

If Option1(0).Value = True Then
    lParam = 0
ElseIf Option1(1).Value = True Then
    lParam = txtSetValue
    If Err.Number <> 0 Then
        MsgBox "Invalid Set Value", vbCritical, "Error"
        Exit Sub
    End If
Else
    lParam = cmbChangeType.ListIndex
End If

txtResult = SendMessage(hWnd, Message, wParam, lParam)

End Sub

Private Sub Form_Load()
Option1_Click 0
cmbChangeType.ListIndex = 0
End Sub

Private Sub Option1_Click(Index As Integer)
If Option1(0).Value = True Then
    txtSetValue.Enabled = False
    cmbChangeType.Enabled = False
ElseIf Option1(1).Value = True Then
    txtSetValue.Enabled = True
    cmbChangeType.Enabled = False
Else
    txtSetValue.Enabled = False
    cmbChangeType.Enabled = True
End If
   
End Sub

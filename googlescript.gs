function doGet(e) {
  const ssId = "1tFvV7p7ays2bCXQSU_jz9ElnlhdpYncXEfIWUApTN_s";  // Replace with your actual spreadsheet ID
  const ss = SpreadsheetApp.openById(ssId);
  const dbSheet = ss.getSheetByName("database");
  const logSheet = ss.getSheetByName("log");

  const rfid = e.parameter.rfid;
  if (!rfid) {
    return ContentService.createTextOutput("Missing RFID");
  }

  const now = new Date();
  const dbData = dbSheet.getDataRange().getValues();
  let found = false;
  let rowIndex = -1;

  for (let i = 1; i < dbData.length; i++) {
    if (dbData[i][0] == rfid) {
      found = true;
      rowIndex = i + 1;
      break;
    }
  }

  let type = "-";
  let value = "-";
  let status = "OUT";

  if (found) {
    type = dbSheet.getRange(rowIndex, 2).getValue();
    value = dbSheet.getRange(rowIndex, 3).getValue();
    status = dbSheet.getRange(rowIndex, 4).getValue();

    status = (status === "IN") ? "OUT" : "IN";
    dbSheet.getRange(rowIndex, 4).setValue(status);
  } else {
    dbSheet.appendRow([rfid, type, value, status]);
  }

  logSheet.appendRow([now, rfid, type, value, status]);

  return ContentService.createTextOutput("Success");
}

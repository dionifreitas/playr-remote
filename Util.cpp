#include "Util.h"

String splitString(String data, char separator, int index) {
  int separatorCount = 0;
  int valueStartIndex = 0;
  int valueEndIndex = -1;
  int lastIndex = data.length() - 1;

  for (int i = 0; i <= lastIndex && separatorCount <= index; i++) {
    if (data.charAt(i) == separator || i == lastIndex) {
      separatorCount++;
      valueStartIndex = valueEndIndex + 1;
      valueEndIndex = (i == lastIndex) ? i + 1 : i;
    }
  }

  if (separatorCount > index) {
    return data.substring(valueStartIndex, valueEndIndex);
  } else {
    return "";
  }
}

String trim(const String& str) {
  int startIndex = 0;
  int endIndex = str.length() - 1;

  while (startIndex <= endIndex && isWhitespace(str[startIndex])) {
    startIndex++;
  }

  while (endIndex >= startIndex && isWhitespace(str[endIndex])) {
    endIndex--;
  }

  return str.substring(startIndex, endIndex + 1);
}
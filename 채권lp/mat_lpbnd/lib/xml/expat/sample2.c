


enum XML_Status XMLCALL
XML_Parse(XML_Parser parser, const char *s, int len, int isFinal)
{
  if ((parser == NULL) || (len < 0) || ((s == NULL) && (len != 0))) {
    if (parser != NULL)
      parser->m_errorCode = XML_ERROR_INVALID_ARGUMENT;
    return XML_STATUS_ERROR;
  }
  switch (parser->m_parsingStatus.parsing) {
  }
}




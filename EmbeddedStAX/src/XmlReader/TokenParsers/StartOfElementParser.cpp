/*
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <http://unlicense.org>
 */

#include <EmbeddedStAX/XmlReader/TokenParsers/StartOfElementParser.h>
#include <EmbeddedStAX/XmlValidator/Common.h>
#include <EmbeddedStAX/XmlValidator/Name.h>

using namespace EmbeddedStAX::XmlReader;

/**
 * Constructor
 */
StartOfElementParser::StartOfElementParser()
    : AbstractTokenParser(ParserType_Reference),
      m_state(State_ReadingElementName),
      m_nameParser(),
      m_attributeValueParser(),
      m_elementName(),
      m_attributeName(),
      m_attributeList()
{
}

/**
 * Destructor
 */
StartOfElementParser::~StartOfElementParser()
{
}

/**
 * Get element name
 *
 * \return Element name
 */
EmbeddedStAX::Common::UnicodeString StartOfElementParser::name() const
{
    return m_elementName;
}

/**
 * Get attribute list
 *
 * \return Attribute list
 */
const EmbeddedStAX::Common::AttributeList &StartOfElementParser::attributeList() const
{
    return m_attributeList;
}

/**
 * Parse
 *
 * \retval Result_Success       Success
 * \retval Result_NeedMoreData  More data is needed
 * \retval Result_Error         Error
 */
AbstractTokenParser::Result StartOfElementParser::parse()
{
    Result result = Result_Error;

    if (isInitialized())
    {
        bool finishParsing = false;

        while (!finishParsing)
        {
            finishParsing = true;
            State nextState = State_Error;

            switch (m_state)
            {
                case State_ReadingElementName:
                {
                    // Reading element name
                    nextState = executeStateReadingElementName();

                    // Check transitions
                    switch (nextState)
                    {
                        case State_ReadingElementName:
                        {
                            result = Result_NeedMoreData;
                            break;
                        }

                        case State_ReadingNextItem:
                        case State_ReadingEndOfEmptyElement:
                        {
                            // Execute another cycle
                            finishParsing = false;
                            break;
                        }

                        case State_Finished:
                        {
                            result = Result_Success;
                            break;
                        }

                        default:
                        {
                            // Error
                            nextState = State_Error;
                            break;
                        }
                    }
                    break;
                }

                case State_ReadingNextItem:
                {
                    // Reading next item
                    nextState = executeStateReadingNextItem();

                    // Check transitions
                    switch (nextState)
                    {
                        case State_ReadingNextItem:
                        {
                            result = Result_NeedMoreData;
                            break;
                        }

                        case State_ReadingAttributeName:
                        case State_ReadingElementName:
                        case State_ReadingEndOfEmptyElement:
                        {
                            // Execute another cycle
                            finishParsing = false;
                            break;
                        }

                        case State_Finished:
                        {
                            result = Result_Success;
                            break;
                        }

                        default:
                        {
                            // Error
                            nextState = State_Error;
                            break;
                        }
                    }
                    break;
                }

                case State_ReadingAttributeName:
                {
                    // Reading attribute name
                    nextState = executeStateReadingAttributeName();

                    // Check transitions
                    switch (nextState)
                    {
                        case State_ReadingAttributeName:
                        {
                            result = Result_NeedMoreData;
                            break;
                        }

                        case State_ReadingEqualSign:
                        case State_ReadingEndOfEmptyElement:
                        {
                            // Execute another cycle
                            finishParsing = false;
                            break;
                        }

                        case State_Finished:
                        {
                            result = Result_Success;
                            break;
                        }

                        default:
                        {
                            // Error
                            nextState = State_Error;
                            break;
                        }
                    }
                    break;
                }

                case State_ReadingEqualSign:
                {
                    // Reading equal sign
                    nextState = executeStateReadingEqualSign();

                    // Check transitions
                    switch (nextState)
                    {
                        case State_ReadingEqualSign:
                        {
                            result = Result_NeedMoreData;
                            break;
                        }

                        case State_ReadingAttributeValue:
                        {
                            // Execute another cycle
                            finishParsing = false;
                            break;
                        }

                        default:
                        {
                            // Error
                            nextState = State_Error;
                            break;
                        }
                    }
                    break;
                }

                case State_ReadingAttributeValue:
                {
                    // Reading attribute value
                    nextState = executeStateReadingAttributeValue();

                    // Check transitions
                    switch (nextState)
                    {
                        case State_ReadingAttributeValue:
                        {
                            result = Result_NeedMoreData;
                            break;
                        }

                        case State_ReadingNextItem:
                        {
                            // Execute another cycle
                            finishParsing = false;
                            break;
                        }

                        default:
                        {
                            // Error
                            nextState = State_Error;
                            break;
                        }
                    }
                    break;
                }

                case State_ReadingEndOfEmptyElement:
                {
                    // Reading end of empty element
                    nextState = executeStateReadingEndOfEmptyElement();

                    // Check transitions
                    switch (nextState)
                    {
                        case State_ReadingEndOfEmptyElement:
                        {
                            result = Result_NeedMoreData;
                            break;
                        }

                        case State_Finished:
                        {
                            result = Result_Success;
                            break;
                        }

                        default:
                        {
                            // Error
                            nextState = State_Error;
                            break;
                        }
                    }
                    break;
                }

                case State_Finished:
                {
                    result = Result_Success;
                    break;
                }

                default:
                {
                    // Error, invalid state
                    nextState = State_Error;
                    break;
                }
            }

            // Update state
            m_state = nextState;
        }
    }

    if ((result == Result_Success) ||
        (result == Result_Error))
    {
        parsingBuffer()->eraseToCurrentPosition();
    }

    return result;
}

/**
 * Initialize parser's additional data
 *
 * \retval true     Success
 * \retval false    Error
 */
bool StartOfElementParser::initializeAdditionalData()
{
    m_state = State_ReadingElementName;
    m_elementName.clear();
    m_attributeName.clear();
    m_attributeList.clear();
    parsingBuffer()->eraseToCurrentPosition();
    m_attributeValueParser.deinitialize();

    return m_nameParser.initialize(parsingBuffer());
}

/**
 * Deinitialize parser's additional data
 */
void StartOfElementParser::deinitializeAdditionalData()
{
    m_state = State_ReadingElementName;
    m_elementName.clear();
    m_attributeName.clear();
    m_attributeList.clear();
    m_nameParser.deinitialize();
    m_attributeValueParser.deinitialize();
}

/**
 * Execute state: Reading element name
 *
 * \retval State_ReadingElementName         Wait for more data
 * \retval State_ReadingNextItem            Whitespace found
 * \retval State_ReadingEndOfEmptyElement   End of empty element found
 * \retval State_Finished                   End of start of element found
 * \retval State_Error                      Error, unexpected character
 *
 * Format:
 * \code{.unparsed}
 * STag         ::= '<' Name (S Attribute)* S? '>'
 * EmptyElemTag	::= '<' Name (S Attribute)* S? '/>'
 * \endcode
 */
StartOfElementParser::State StartOfElementParser::executeStateReadingElementName()
{
    State nextState = State_Error;

    // Parse
    const Result result = m_nameParser.parse();

    switch (result)
    {
        case Result_NeedMoreData:
        {
            // More data is needed
            nextState = State_ReadingElementName;
            break;
        }

        case Result_Success:
        {
            const uint32_t uchar = parsingBuffer()->currentChar();

            if (uchar == static_cast<uint32_t>('>'))
            {
                // End of start of element found
                m_elementName = m_nameParser.value();
                m_attributeList.clear();
                parsingBuffer()->incrementPosition();
                parsingBuffer()->eraseToCurrentPosition();
                setTokenType(TokenType_StartOfElement);
                nextState = State_Finished;
            }
            else if (uchar == static_cast<uint32_t>('/'))
            {
                // End of empty element found
                m_elementName = m_nameParser.value();
                m_attributeList.clear();
                parsingBuffer()->incrementPosition();
                parsingBuffer()->eraseToCurrentPosition();
                nextState = State_ReadingEndOfEmptyElement;
            }
            else if (XmlValidator::isWhitespace(uchar))
            {
                // End of element name, start reading next item
                m_elementName = m_nameParser.value();
                m_attributeList.clear();
                parsingBuffer()->incrementPosition();
                parsingBuffer()->eraseToCurrentPosition();
                nextState = State_ReadingNextItem;
            }
            else
            {
                // Error, invalid character
                setTerminationChar(uchar);
            }

            m_nameParser.deinitialize();
            break;
        }

        default:
        {
            // Error
            m_nameParser.deinitialize();
            break;
        }
    }

    return nextState;
}

/**
 * Execute state: Reading next item
 *
 * \retval State_ReadingNextItem            Wait for more data
 * \retval State_ReadingAttributeName       Start of attribute name found
 * \retval State_ReadingEndOfEmptyElement   End of empty element found
 * \retval State_ReadingFinished            End of start of element found
 * \retval State_Error                      Error, unexpected character
 */
StartOfElementParser::State StartOfElementParser::executeStateReadingNextItem()
{
    State nextState = State_Error;
    bool finishParsing = false;

    while (!finishParsing)
    {
        finishParsing = true;

        // Check if more data is needed
        if (parsingBuffer()->isMoreDataNeeded())
        {
            // More data is needed
            nextState = State_ReadingNextItem;
        }
        else
        {
            // Check character
            const uint32_t uchar = parsingBuffer()->currentChar();

            if (uchar == static_cast<uint32_t>('>'))
            {
                // End of start of element found
                parsingBuffer()->incrementPosition();
                parsingBuffer()->eraseToCurrentPosition();
                setTokenType(TokenType_StartOfElement);
                nextState = State_Finished;
            }
            else if (uchar == static_cast<uint32_t>('/'))
            {
                // End of empty element found
                parsingBuffer()->incrementPosition();
                parsingBuffer()->eraseToCurrentPosition();
                nextState = State_ReadingEndOfEmptyElement;
            }
            else if (XmlValidator::isNameStartChar(uchar))
            {
                // Start of attribute name found, start reading the next attribute
                parsingBuffer()->eraseToCurrentPosition();
                m_nameParser.initialize(parsingBuffer());
                nextState = State_ReadingAttributeName;
            }
            else if (XmlValidator::isWhitespace(uchar))
            {
                // Whitespace found, ignore it
                parsingBuffer()->incrementPosition();
                parsingBuffer()->eraseToCurrentPosition();
                finishParsing = false;
            }
            else
            {
                // Error, invalid character
                setTerminationChar(uchar);
            }
        }
    }

    return nextState;
}

/**
 * Execute state: Reading attribute name
 *
 * \retval State_ReadingAttributeName       Wait for more data
 * \retval State_ReadingEqualSign           End of attribute name found
 * \retval State_ReadingEndOfEmptyElement   End of empty element found
 * \retval State_Finished                   End of start of element found
 * \retval State_Error                      Error, unexpected character
 */
StartOfElementParser::State StartOfElementParser::executeStateReadingAttributeName()
{
    State nextState = State_Error;

    // Parse
    const Result result = m_nameParser.parse();

    switch (result)
    {
        case Result_NeedMoreData:
        {
            // More data is needed
            nextState = State_ReadingAttributeName;
            break;
        }

        case Result_Success:
        {
            // End of attribute name found
            m_attributeName = m_nameParser.value();
            m_nameParser.deinitialize();
            nextState = State_ReadingEqualSign;
            break;
        }

        case Result_Error:
        {
            // Check for end of entity reference
            const uint32_t terminationChar = m_nameParser.terminationChar();
            m_nameParser.deinitialize();

            if (terminationChar == static_cast<uint32_t>('>'))
            {
                // End of start of element found
                parsingBuffer()->incrementPosition();
                parsingBuffer()->eraseToCurrentPosition();
                setTokenType(TokenType_StartOfElement);
                nextState = State_Finished;
            }
            else if (terminationChar == static_cast<uint32_t>('/'))
            {
                // End of empty element found
                parsingBuffer()->incrementPosition();
                parsingBuffer()->eraseToCurrentPosition();
                nextState = State_ReadingEndOfEmptyElement;
            }
            else
            {
                // Error, invalid character
                setTerminationChar(terminationChar);
            }
            break;
        }

        default:
        {
            // Should never be reached! (defensive programing)
            break;
        }
    }

    return nextState;
}

/**
 * Execute state: Reading equal sign
 *
 * \retval State_ReadingEqualSign       Wait for more data
 * \retval State_ReadingAttributeValue  Equal sign found
 * \retval State_Error                  Error, unexpected character
 *
 * Format:
 * \code{.unparsed}
 * Equal sign ::= S? '='
 * \endcode
 */
StartOfElementParser::State StartOfElementParser::executeStateReadingEqualSign()
{
    State nextState = State_Error;
    bool finishParsing = false;

    while (!finishParsing)
    {
        finishParsing = true;

        // Check if more data is needed
        if (parsingBuffer()->isMoreDataNeeded())
        {
            // More data is needed
            nextState = State_ReadingEqualSign;
        }
        else
        {
            // Check character
            const uint32_t uchar = parsingBuffer()->currentChar();

            if (uchar == static_cast<uint32_t>('='))
            {
                // Equal sign found
                parsingBuffer()->incrementPosition();
                parsingBuffer()->eraseToCurrentPosition();
                m_attributeValueParser.initialize(parsingBuffer(), Option_IgnoreLeadingWhitespace);
                nextState = State_ReadingAttributeValue;
            }
            else if (XmlValidator::isWhitespace(uchar))
            {
                // Ignore leading whitespace
                parsingBuffer()->incrementPosition();
                finishParsing = false;
            }
            else
            {
                // Error, invalid character
                setTerminationChar(uchar);
            }
        }
    }

    return nextState;
}

/**
 * Execute state: Reading attribute value
 *
 * \retval State_ReadingAttributeValue  Wait for more data
 * \retval State_ReadingNextAttribute   Attribute value found
 * \retval State_Error                  Error, unexpected character
 */
StartOfElementParser::State StartOfElementParser::executeStateReadingAttributeValue()
{
    State nextState = State_Error;

    // Parse
    const Result result = m_attributeValueParser.parse();

    switch (result)
    {
        case Result_NeedMoreData:
        {
            // More data is needed
            nextState = State_ReadingAttributeValue;
            break;
        }

        case Result_Success:
        {
            // Add attribute to the attribute list
            const Common::Attribute attribute(m_attributeName, m_attributeValueParser.value());
            m_attributeList.add(attribute);
            m_attributeName.clear();
            m_attributeValueParser.deinitialize();
            nextState = State_ReadingNextItem;
            break;
        }

        default:
        {
            // Error
            m_attributeValueParser.deinitialize();
            break;
        }
    }

    return nextState;
}

/**
 * Execute state: Reading end of empty element
 *
 * \retval State_ReadingEndOfEmptyElement   Wait for more data
 * \retval State_ReadingFinished            End of empty element found
 * \retval State_Error                      Error, unexpected character
 */
StartOfElementParser::State StartOfElementParser::executeStateReadingEndOfEmptyElement()
{
    State nextState = State_Error;

    // Check if more data is needed
    if (parsingBuffer()->isMoreDataNeeded())
    {
        // More data is needed
        nextState = State_ReadingEndOfEmptyElement;
    }
    else
    {
        // Check character
        const uint32_t uchar = parsingBuffer()->currentChar();

        if (uchar == static_cast<uint32_t>('>'))
        {
            // End of empty element found
            parsingBuffer()->incrementPosition();
            parsingBuffer()->eraseToCurrentPosition();
            setTokenType(TokenType_EmptyElement);
            nextState = State_Finished;
        }
        else
        {
            // Error, invalid character read
            setTerminationChar(uchar);
        }
    }

    return nextState;
}

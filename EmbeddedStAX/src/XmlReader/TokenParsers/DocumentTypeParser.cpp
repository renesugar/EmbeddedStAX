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

#include <EmbeddedStAX/XmlReader/TokenParsers/DocumentTypeParser.h>
#include <EmbeddedStAX/XmlValidator/Common.h>
#include <EmbeddedStAX/XmlValidator/Name.h>

using namespace EmbeddedStAX::XmlReader;

/**
 * Constructor
 */
DocumentTypeParser::DocumentTypeParser()
    : AbstractTokenParser(ParserType_DocumentType),
      m_state(State_ReadingName),
      m_nameParser(),
      m_documentType()
{
}

/**
 * Destructor
 */
DocumentTypeParser::~DocumentTypeParser()
{
}

/**
 * Get processing instruction
 *
 * \return Processing instruction
 */
EmbeddedStAX::Common::DocumentType DocumentTypeParser::documentType() const
{
    return m_documentType;
}

/**
 * Parse
 *
 * \retval Result_Success       Success
 * \retval Result_NeedMoreData  More data is needed
 * \retval Result_Error         Error
 */
AbstractTokenParser::Result DocumentTypeParser::parse()
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
                case State_ReadingName:
                {
                    // Reading name of the root element
                    nextState = executeStateReadingName();

                    // Check transitions
                    switch (nextState)
                    {
                        case State_ReadingName:
                        {
                            result = Result_NeedMoreData;
                            break;
                        }

                        case State_ReadingEnd:
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

                case State_ReadingEnd:
                {
                    // Reading end of document type
                    nextState = executeStateReadingEnd();

                    // Check transitions
                    switch (nextState)
                    {
                        case State_ReadingEnd:
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
bool DocumentTypeParser::initializeAdditionalData()
{
    m_state = State_ReadingName;
    m_documentType.clear();
    parsingBuffer()->eraseToCurrentPosition();

    return m_nameParser.initialize(parsingBuffer(), Option_IgnoreLeadingWhitespace);
}

/**
 * Deinitialize parser's additional data
 */
void DocumentTypeParser::deinitializeAdditionalData()
{
    m_state = State_ReadingName;
    m_documentType.clear();
    m_nameParser.deinitialize();
}

/**
 * Execute state: Reading name of the root element
 *
 * \retval State_ReadingName    Wait for more data
 * \retval State_ReadingEnd     Document type read
 * \retval State_Error          Error
 */
DocumentTypeParser::State DocumentTypeParser::executeStateReadingName()
{
    State nextState = State_Error;
    bool finishParsing = false;

    while (!finishParsing)
    {
        finishParsing = true;

        // Parse
        const Result result = m_nameParser.parse();

        switch (result)
        {
            case Result_NeedMoreData:
            {
                // More data is needed
                nextState = State_ReadingName;
                break;
            }

            case Result_Success:
            {
                m_documentType.setName(m_nameParser.value());
                m_nameParser.deinitialize();

                // Read end of document type
                nextState = State_ReadingEnd;
                break;
            }

            default:
            {
                // Error
                m_nameParser.deinitialize();
                break;
            }
        }
    }

    return nextState;
}

/**
 * Execute state: Reading end of document type
 *
 * \retval State_ReadingEnd Wait for more data
 * \retval State_Finished   End of document type read
 * \retval State_Error      Error
 */
DocumentTypeParser::State DocumentTypeParser::executeStateReadingEnd()
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
            nextState = State_ReadingEnd;
        }
        else
        {
            // Check character
            const uint32_t uchar = parsingBuffer()->currentChar();

            if (uchar == static_cast<uint32_t>('>'))
            {
                parsingBuffer()->incrementPosition();
                parsingBuffer()->eraseToCurrentPosition();

                if (m_documentType.isValid())
                {
                    // End of document type found
                    nextState = State_Finished;
                }
                else
                {
                    // Error, invalid document type
                }
            }
            else if (XmlValidator::isWhitespace(uchar))
            {
                // We are allowed to ignore whitespace characters
                parsingBuffer()->incrementPosition();
                parsingBuffer()->eraseToCurrentPosition();
                finishParsing = false;
            }
            else
            {
                // Error, invalid character read
            }
        }
    }

    return nextState;
}

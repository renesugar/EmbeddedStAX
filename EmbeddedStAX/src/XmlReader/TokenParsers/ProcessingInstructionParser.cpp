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

#include <EmbeddedStAX/XmlReader/TokenParsers/ProcessingInstructionParser.h>
#include <EmbeddedStAX/XmlValidator/Common.h>
#include <EmbeddedStAX/XmlValidator/ProcessingInstruction.h>

using namespace EmbeddedStAX::XmlReader;

/**
 * Constructor
 */
ProcessingInstructionParser::ProcessingInstructionParser()
    : AbstractTokenParser(ParserType_ProcessingInstruction),
      m_state(State_ReadingPiTarget),
      m_nameParser(),
      m_processingInstruction(),
      m_xmlDeclaration()
{
}

/**
 * Destructor
 */
ProcessingInstructionParser::~ProcessingInstructionParser()
{
}

/**
 * Get processing instruction
 *
 * \return Processing instruction
 */
EmbeddedStAX::Common::ProcessingInstruction
ProcessingInstructionParser::processingInstruction() const
{
    return m_processingInstruction;
}

/**
 * Get XML declaration
 *
 * \return XML declaration
 */
EmbeddedStAX::Common::XmlDeclaration ProcessingInstructionParser::xmlDeclaration() const
{
    return m_xmlDeclaration;
}

/**
 * Parse
 *
 * \retval Result_Success       Success
 * \retval Result_NeedMoreData  More data is needed
 * \retval Result_Error         Error
 */
AbstractTokenParser::Result ProcessingInstructionParser::parse()
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
                case State_ReadingPiTarget:
                {
                    // Reading processing instruction target (name of the PI)
                    nextState = executeStateReadingPiTarget();

                    // Check transitions
                    switch (nextState)
                    {
                        case State_ReadingPiTarget:
                        {
                            result = Result_NeedMoreData;
                            break;
                        }

                        case State_ReadingPiData:
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

                case State_ReadingPiData:
                {
                    // Reading processing instruction data
                    nextState = executeStateReadingPiData();

                    // Check transitions
                    switch (nextState)
                    {
                        case State_ReadingPiData:
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
bool ProcessingInstructionParser::initializeAdditionalData()
{
    m_state = State_ReadingPiTarget;
    m_processingInstruction.clear();
    m_xmlDeclaration.clear();
    parsingBuffer()->eraseToCurrentPosition();

    return m_nameParser.initialize(parsingBuffer());
}

/**
 * Deinitialize parser's additional data
 */
void ProcessingInstructionParser::deinitializeAdditionalData()
{
    m_state = State_ReadingPiTarget;
    m_processingInstruction.clear();
    m_xmlDeclaration.clear();
    m_nameParser.deinitialize();
}

/**
 * Execute state: Reading PITarget
 *
 * \retval State_ReadingPiTarget        Wait for more data
 * \retval State_ReadingPiData          PITarget read
 * \retval State_Error                  Error
 */
ProcessingInstructionParser::State ProcessingInstructionParser::executeStateReadingPiTarget()
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
                nextState = State_ReadingPiTarget;
                break;
            }

            case Result_Success:
            {
                m_piTarget = m_nameParser.value();
                m_nameParser.deinitialize();
                nextState = State_ReadingPiData;
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
 * Execute state: Reading PI Data
 *
 * \retval State_ReadingPiData  Wait for more data
 * \retval State_Finished       Processing instruction or XML declaration read
 * \retval State_Error          Error
 *
 * Format:
 * \code{.unparsed}
 * PI Data ::= (Char* - (Char* '?>' Char*))
 * \endcode
 */
ProcessingInstructionParser::State ProcessingInstructionParser::executeStateReadingPiData()
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
            nextState = State_ReadingPiData;
        }
        else
        {
            // Check character
            const uint32_t uchar = parsingBuffer()->currentChar();

            if (XmlValidator::isChar(uchar))
            {
                // Check for "?>" sequence
                const size_t currentPosition = parsingBuffer()->currentPosition();

                if (currentPosition == 0U)
                {
                    // Check next character
                    parsingBuffer()->incrementPosition();
                    finishParsing = false;
                }
                else
                {
                    if ((uchar == static_cast<uint32_t>('>')) &&
                        (parsingBuffer()->at(currentPosition - 1U) == static_cast<uint32_t>('?')))
                    {
                        // End of PI Data found
                        const Common::UnicodeString piData =
                                parsingBuffer()->substring(0U, currentPosition - 1U);
                        parsingBuffer()->incrementPosition();
                        parsingBuffer()->eraseToCurrentPosition();

                        // Check for XML declaration
                        if (XmlValidator::isXmlDeclaration(m_piTarget))
                        {
                            // Parse XML declaration
                            m_xmlDeclaration = Common::XmlDeclaration::fromPiData(piData);

                            if (m_xmlDeclaration.isValid())
                            {
                                // XML declaration read
                                setTokenType(TokenType_XmlDeclaration);
                                nextState = State_Finished;
                            }
                        }
                        else
                        {
                            m_processingInstruction.setPiTarget(m_piTarget);
                            m_processingInstruction.setPiData(piData);

                            if (m_processingInstruction.isValid())
                            {
                                // Processing instruction read
                                setTokenType(TokenType_ProcessingInstruction);
                                nextState = State_Finished;
                            }
                            else
                            {
                                // Error
                                m_processingInstruction.clear();
                            }
                        }
                    }
                    else
                    {
                        // Check next character
                        parsingBuffer()->incrementPosition();
                        finishParsing = false;
                    }
                }
            }
            else
            {
                // Error, invalid character read
                setTerminationChar(uchar);
            }
        }
    }

    return nextState;
}

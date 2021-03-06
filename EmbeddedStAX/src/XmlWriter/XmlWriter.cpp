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

#include <EmbeddedStAX/XmlWriter/XmlWriter.h>
#include <EmbeddedStAX/XmlValidator/Attribute.h>
#include <EmbeddedStAX/XmlValidator/CDataSection.h>
#include <EmbeddedStAX/XmlValidator/Comment.h>
#include <EmbeddedStAX/XmlValidator/Name.h>
#include <EmbeddedStAX/XmlValidator/Reference.h>
#include <EmbeddedStAX/XmlValidator/TextNode.h>
//#include <Common/Common.h>

using namespace EmbeddedStAX;

/**
 * Constructor
 */
XmlWriter::XmlWriter::XmlWriter()
{
    clearDocument();
}

/**
 * Clear XML document
 */
void XmlWriter::XmlWriter::clearDocument()
{
    m_state = State_Empty;
    m_documentType;
    m_openedElementList.clear();
    m_xmlString.clear();
}

/**
 * Get XML string
 *
 * \return XML string
 * \return Empty string on error
 */
Common::UnicodeString XmlWriter::XmlWriter::xmlString() const
{
    return m_xmlString;
}

/**
 * Write XML Declaration in the XML document
 *
 * \retval true     Success
 * \retval false    Error, XML document is not empty
 *
 * \note Currently only XML Declaration with parameters version 1.0 with encoding "UTF-8" are
 *       supported!
 */
bool XmlWriter::XmlWriter::writeXmlDeclaration()
{
    bool success = false;

    if (m_state == State_Empty)
    {
        // TODO: add the "standalone" attribute?

        // Set XML Declaration
        m_xmlString = Common::Utf8::toUnicodeString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
        m_state = State_DocumentStarted;
        success = true;
    }
    else
    {
        // Error, invalid state
        m_state = State_Error;
    }

    return success;
}

/**
 * Write Document Type in the XML document
 *
 * \param documentType  Document type name
 *
 * \retval true     Success
 * \retval false    Error
 *
 * \note If the document type name is set then the root element name must match the document type
 *       name!
 */
bool XmlWriter::XmlWriter::writeDocumentType(const Common::UnicodeString &documentType)
{
    bool success = false;

    if (m_documentType.empty())
    {
        if ((m_state == State_Empty) ||
            (m_state == State_DocumentStarted))
        {
            if (XmlValidator::validateName(documentType))
            {
                // Create Document Type
                m_xmlString.append(Common::Utf8::toUnicodeString("<!DOCTYPE "));
                m_xmlString.append(documentType);
                m_xmlString.push_back(static_cast<uint32_t>('>'));

                m_documentType = documentType;
                m_state = State_DocumentStarted;
                success = true;
            }
        }
    }

    if (!success)
    {
        // Error
        m_state = State_Error;
    }

    return success;
}

/**
 * Write Comment in the XML document
 *
 * \param commentText   Text that should be written to the comment
 *
 * \retval true     Success
 * \retval false    Error
 */
bool XmlWriter::XmlWriter::writeComment(const Common::UnicodeString &commentText)
{
    bool success = false;
    State nextState = m_state;

    if (XmlValidator::validateCommentText(commentText))
    {
        switch (m_state)
        {
            case State_Empty:
            {
                nextState = State_DocumentStarted;
                success = true;
                break;
            }

            case State_DocumentStarted:
            case State_Element:
            case State_DocumentEnded:
            {
                success = true;
                break;
            }

            default:
            {
                // Error, invalid state
                break;
            }
        }
    }

    if (success)
    {
        // Write Comment
        m_xmlString.append(Common::Utf8::toUnicodeString("<!--"));
        m_xmlString.append(commentText);
        m_xmlString.append(Common::Utf8::toUnicodeString("-->"));
        m_state = nextState;
    }
    else
    {
        // Error
        m_state = State_Error;
    }

    return success;
}

/**
 * Write Processing Instruction in the XML document
 *
 * \param piTarget  Processing instruction target name
 * \param piValue   Processing instruction value
 *
 * \retval true     Success
 * \retval false    Error
 */
bool XmlWriter::XmlWriter::writeProcessingInstruction(const Common::ProcessingInstruction &pi)
{
    bool success = false;
    State nextState = m_state;

    if (pi.isValid())
    {
        switch (m_state)
        {
            case State_Empty:
            {
                nextState = State_DocumentStarted;
                success = true;
                break;
            }

            case State_DocumentStarted:
            case State_Element:
            case State_DocumentEnded:
            {
                success = true;
                break;
            }

            default:
            {
                // Error, invalid state
                break;
            }
        }
    }

    if (success)
    {
        // Write Processing Instruction
        m_xmlString.append(Common::Utf8::toUnicodeString("<?"));
        m_xmlString.append(pi.piTarget());

        const Common::UnicodeString piData = pi.piData();

        if (!piData.empty())
        {
            m_xmlString.push_back(static_cast<uint32_t>(' '));
            m_xmlString.append(piData);
        }

        m_xmlString.append(Common::Utf8::toUnicodeString("?>"));
        m_state = nextState;
    }
    else
    {
        // Error
        m_state = State_Error;
    }

    return success;
}

/**
 * Write Empty Element in the XML document
 *
 * \param elementName   Element name
 * \param attributeList Attribute list
 *
 * \retval true     Success
 * \retval false    Error
 *
 * \note If Document Type is set then the root element name must match it!
 */
bool XmlWriter::XmlWriter::writeEmptyElement(const Common::UnicodeString &elementName,
                                             const Common::AttributeList &attributeList)
{
    bool success = false;
    State nextState = State_Error;

    if (XmlValidator::validateName(elementName))
    {
        switch (m_state)
        {
            case State_Empty:
            case State_DocumentStarted:
            {
                // Check for root element
                if (m_openedElementList.size() == 0U)
                {
                    // Root element
                    // No validation of root element name is required if document type is not set
                    if (m_documentType.empty())
                    {
                        nextState = State_DocumentEnded;
                        success = true;
                    }
                    // Check if element name matches the document type
                    else if (elementName == m_documentType)
                    {
                        nextState = State_DocumentEnded;
                        success = true;
                    }
                    else
                    {
                        // Error, invalid root element name
                    }
                }
                else
                {
                    // Error, there should be no open elements in the current state
                }
                break;
            }

            case State_Element:
            {
                // Child element
                success = true;
                nextState = State_Element;
                break;
            }

            default:
            {
                // Error, invalid state
                break;
            }
        }
    }

    if (success)
    {
        // Write Start of Element
        m_xmlString.push_back(static_cast<uint32_t>('<'));
        m_xmlString.append(elementName);

        // Write Attributes
        success = writeAttributeList(attributeList);

        // Write end of Empty Element
        if (success)
        {
            m_xmlString.append(Common::Utf8::toUnicodeString("/>"));
            m_state = nextState;
        }
    }

    if (!success)
    {
        // Error
        m_state = State_Error;
    }

    return success;
}

/**
 * Write Start of Element in the XML document
 *
 * \param elementName   Element name
 * \param attributeList Attribute list
 *
 * \retval true     Success
 * \retval false    Error
 *
 * \note If Document Type is set then the root element name must match it!
 */
bool XmlWriter::XmlWriter::writeStartOfElement(const Common::UnicodeString &elementName,
                                               const Common::AttributeList &attributeList)
{
    bool success = false;

    if (XmlValidator::validateName(elementName))
    {
        switch (m_state)
        {
            case State_Empty:
            case State_DocumentStarted:
            {
                // Check for root element
                if (m_openedElementList.empty())
                {
                    // Root element
                    // No validation of root element name is required if document type is not set
                    if (m_documentType.empty())
                    {
                        success = true;
                    }
                    // Check if element name matches the document type
                    else if (elementName == m_documentType)
                    {
                        success = true;
                    }
                    else
                    {
                        // Error, invalid root element name
                    }
                }
                else
                {
                    // Error, there should be no open elements in the current state
                }
                break;
            }

            case State_Element:
            {
                // Child element
                success = true;
                break;
            }

            default:
            {
                // Error, invalid state
                break;
            }
        }
    }

    if (success)
    {
        // Write Start of Element
        m_xmlString.push_back(static_cast<uint32_t>('<'));
        m_xmlString.append(elementName);

        // Write Attributes
        success = writeAttributeList(attributeList);

        // Write end of Start of Element
        if (success)
        {
            m_xmlString.push_back(static_cast<uint32_t>('>'));

            m_openedElementList.push_back(elementName);
            m_state = State_Element;
        }
    }

    if (!success)
    {
        // Error
        m_state = State_Error;
    }

    return success;
}

/**
 * Write Text Node
 *
 * \param textNode  Text node
 *
 * \retval true     Success
 * \retval false    Error
 */
bool XmlWriter::XmlWriter::writeTextNode(const Common::UnicodeString &text)
{
    bool success = false;

    if (m_state == State_Element)
    {
        const Common::UnicodeString escapedText = escapeTextNode(text);

        if (XmlValidator::validateTextNode(escapedText))
        {
            m_xmlString.append(escapedText);
            success = true;
        }
        else
        {
            // Error, invalid text
        }
    }
    else
    {
        // Error, invalid state
    }

    if (!success)
    {
        // Error
        m_state = State_Error;
    }

    return success;
}

/**
 * Write CDATA Section
 *
 * \param cdata     CDATA text
 *
 * \retval true     Success
 * \retval false    Error
 */
bool XmlWriter::XmlWriter::writeCDataSection(const Common::UnicodeString &cdata)
{
    bool success = false;

    if (m_state == State_Element)
    {
        if (XmlValidator::validateCDataSection(cdata))
        {
            m_xmlString.append(Common::Utf8::toUnicodeString("<![CDATA["));
            m_xmlString.append(cdata);
            m_xmlString.append(Common::Utf8::toUnicodeString("]]>"));
            success = true;
        }
    }
    else
    {
        // Error, invalid state
    }

    if (!success)
    {
        // Error
        m_state = State_Error;
    }

    return success;
}

/**
 * Writes end of the currently open element in the XML document
 *
 * \retval true     Success
 * \retval false    Error
 */
bool XmlWriter::XmlWriter::writeEndOfElement()
{
    bool success = false;

    if (m_state == State_Element)
    {
        if (m_openedElementList.empty())
        {
            // Error, there should be at least one open element in the current state
        }
        else
        {
            // Write End of Element
            m_xmlString.append(Common::Utf8::toUnicodeString("</"));
            m_xmlString.append(m_openedElementList.back());
            m_openedElementList.pop_back();
            m_xmlString.push_back(static_cast<uint32_t>('>'));

            // Check for end of root element
            if (m_openedElementList.empty())
            {
                m_state = State_DocumentEnded;
            }

            success = true;
        }
    }

    if (!success)
    {
        // Error
        m_state = State_Error;
    }

    return success;
}

/**
 * Write Attribute List in the XML document
 *
 * \param attributeList Attribute list
 *
 * \retval true     Success
 * \retval false    Error
 */
bool XmlWriter::XmlWriter::writeAttributeList(const Common::AttributeList &attributeList)
{
    bool success = true;

    if (attributeList.size() > 0U)
    {
        for (Common::AttributeList::ConstIterator it = attributeList.begin();
             success && (it != attributeList.end());
             it++)
        {
            success = false;
            const Common::Attribute &attribute = *it;
            const Common::UnicodeString name = attribute.name();
            const Common::QuotationMark quotationMark = attribute.valueQuotationMark();
            const Common::UnicodeString escapedValue = escapeAttributeValue(attribute.value(),
                                                                            quotationMark);

            if (XmlValidator::validateName(name) &&
                XmlValidator::validateAttributeValue(escapedValue, quotationMark) &&
                ((quotationMark == Common::QuotationMark_Quote) ||
                 (quotationMark == Common::QuotationMark_Apostrophe)))
            {
                uint32_t quoteChar = static_cast<uint32_t>('"');

                if (attribute.valueQuotationMark() == Common::QuotationMark_Apostrophe)
                {
                    quoteChar = static_cast<uint32_t>('\'');
                }

                m_xmlString.push_back(static_cast<uint32_t>(' '));
                m_xmlString.append(name);
                m_xmlString.push_back(static_cast<uint32_t>('='));
                m_xmlString.push_back(quoteChar);
                m_xmlString.append(escapedValue);
                m_xmlString.push_back(quoteChar);
                success = true;
            }
        }
    }

    return success;
}

/**
 * Esacpe attribute value (if needed)
 *
 * \param attributeValue    Attribute value to escape
 * \param quotationMark     Attribute value's quotation mark
 *
 * \return Escaped string
 */
Common::UnicodeString XmlWriter::XmlWriter::escapeAttributeValue(
        const Common::UnicodeString &attributeValue,
        const Common::QuotationMark quotationMark) const
{
    Common::UnicodeString escapedValue;
    escapedValue.reserve(attributeValue.size());

    bool finished = false;
    size_t position = 0U;

    while ((position < attributeValue.size()) && !finished)
    {
        // Check if valid attribute value character
        const uint32_t uchar = attributeValue.at(position);

        switch (uchar)
        {
            case static_cast<uint32_t>('<'):
            {
                // Escape '<' character
                escapedValue.append(Common::Utf8::toUnicodeString("&lt;"));
                position++;
                break;
            }

            case static_cast<uint32_t>('"'):
            {
                if (quotationMark == Common::QuotationMark_Quote)
                {
                    // Escape the '"' character
                    escapedValue.append(Common::Utf8::toUnicodeString("&quot;"));
                    finished = true;
                }
                else
                {
                    // Valid character
                    escapedValue.push_back(uchar);
                }

                position++;
                break;
            }

            case static_cast<uint32_t>('\''):
            {
                if (quotationMark == Common::QuotationMark_Apostrophe)
                {
                    // Escape the '\'' character
                    escapedValue.append(Common::Utf8::toUnicodeString("&apos;"));
                    finished = true;
                }
                else
                {
                    // Valid character
                    escapedValue.push_back(uchar);
                }

                position++;
                break;
            }

            case static_cast<uint32_t>('&'):
            {
                // Escape '&' character
                escapedValue.append(Common::Utf8::toUnicodeString("&amp;"));
                position++;
                break;
            }

            default:
            {
                // Valid character
                escapedValue.push_back(uchar);
                position++;
                break;
            }
        }
    }

    return escapedValue;
}

/**
 * Esacpe text node (if needed)
 *
 * \param text  Text to escape
 *
 * \return Escaped string
 */
Common::UnicodeString XmlWriter::XmlWriter::escapeTextNode(const Common::UnicodeString &text) const
{
    Common::UnicodeString escapedValue;
    escapedValue.reserve(text.size());

    bool finished = false;
    size_t position = 0U;

    while ((position < text.size()) && !finished)
    {
        // Check if valid text node character
        const uint32_t uchar = text.at(position);

        switch (uchar)
        {
            case static_cast<uint32_t>('<'):
            {
                // Escape '<' character
                escapedValue.append(Common::Utf8::toUnicodeString("&lt;"));
                position++;
                break;
            }

            case static_cast<uint32_t>('&'):
            {
                // Escape '&' character
                escapedValue.append(Common::Utf8::toUnicodeString("&amp;"));
                position++;
                break;
            }

            case static_cast<uint32_t>('>'):
            {
                // Check if '>' character needs to be escaped (only if part of ']]>' sequence)
                bool escapeChar = false;

                if (position >= 2U)
                {
                    // Check for ']]>' sequence
                    if (Common::compareUnicodeString(position - 2U, text, "]]"))
                    {
                        escapeChar = true;
                    }
                }

                if (escapeChar)
                {
                    // Escape '>' character
                    escapedValue.append(Common::Utf8::toUnicodeString("&gt;"));
                }
                else
                {
                    // Valid character
                    escapedValue.push_back(uchar);
                }
                position++;
                break;
            }

            default:
            {
                // Valid character
                escapedValue.push_back(uchar);
                position++;
                break;
            }
        }
    }

    return escapedValue;
}

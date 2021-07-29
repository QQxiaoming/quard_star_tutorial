(: Name: XML-3ed-Mixed-content :)
(: Written by: Tim Kraska :)
(: CDATA sections, comments and PIs may occur in ANY content.       :)
(:              in XML 1.0 3th edition and older                    :)

<foo>
a <![CDATA[cdata section]]> in mixed content.
a <!-- comment --> in mixed content.
a <?processing instruction?> in mixed content.
</foo>

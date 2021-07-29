(:*******************************************************:)
(: Test: K2-IRIToURIfunc-9                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Pass the range of 0x20 to 0x126(hexa decimals). :)
(:*******************************************************:)
<result>
    {
        iri-to-uri(codepoints-to-string(32 to 294)) (: 0x32 to 0x126 in hexa decimal:)
    }
</result>
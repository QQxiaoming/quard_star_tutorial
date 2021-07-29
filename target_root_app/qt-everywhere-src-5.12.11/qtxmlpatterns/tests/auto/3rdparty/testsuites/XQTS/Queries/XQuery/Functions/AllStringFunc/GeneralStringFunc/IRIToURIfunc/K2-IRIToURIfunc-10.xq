(:*******************************************************:)
(: Test: K2-IRIToURIfunc-10                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Pass a relatively high range of unicode characters. :)
(:*******************************************************:)
<result>
    {
        iri-to-uri(codepoints-to-string(15000 to 16000)) (: 0x32 to 0x126 in hexa decimal:)
    }
</result>
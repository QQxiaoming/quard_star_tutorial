(:*******************************************************:)
(: Test: K2-Serialization-9                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use characters in the range of #x7F through #x9F in an attribute node and ensure they are escaped properly. :)
(:*******************************************************:)
(: #x7F - #x9F :)
<a attr="{codepoints-to-string(127 to 159)}"/>
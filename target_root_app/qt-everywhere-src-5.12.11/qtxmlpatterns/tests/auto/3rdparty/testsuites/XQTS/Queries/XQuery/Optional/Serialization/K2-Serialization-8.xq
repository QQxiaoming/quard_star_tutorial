(:*******************************************************:)
(: Test: K2-Serialization-8                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use characters in an attribute node that in XML 1.0 are invalid, and in XML 1.1 must be escaped. :)
(:*******************************************************:)
(: #x1 - #x1F :)
<a attr="{codepoints-to-string(1 to 31)}"></a>
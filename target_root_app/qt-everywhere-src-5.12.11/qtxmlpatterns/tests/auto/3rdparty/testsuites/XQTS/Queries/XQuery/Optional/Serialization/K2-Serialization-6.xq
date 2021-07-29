(:*******************************************************:)
(: Test: K2-Serialization-6                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that CR, NL, TAB, NEL and LINE SEPARATOR in attributes are escaped when serialized. :)
(:*******************************************************:)
<a attr="&#xD;&#xA;&#x9;&#x85;&#x2028;"/>
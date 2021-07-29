(:*******************************************************:)
(: Test: K2-ComputeConAttr-48                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure xml:id is properly normalized, and not done at the serialization stage. :)
(:*******************************************************:)
string(attribute xml:id {"    ab    c


d       "})
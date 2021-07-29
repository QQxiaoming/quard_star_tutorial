(:*******************************************************:)
(: Test: K2-RoundFunc-2                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure the return type is properly inferred for unsignedLong. :)
(:*******************************************************:)
round(xs:unsignedLong(<e>0</e>)) instance of xs:unsignedLong
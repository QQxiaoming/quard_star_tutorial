(:*******************************************************:)
(: Test: K2-CeilingFunc-8                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure the return type is properly inferred for negativeInteger. :)
(:*******************************************************:)
ceiling(xs:negativeInteger(<e>-1</e>)) instance of xs:negativeInteger
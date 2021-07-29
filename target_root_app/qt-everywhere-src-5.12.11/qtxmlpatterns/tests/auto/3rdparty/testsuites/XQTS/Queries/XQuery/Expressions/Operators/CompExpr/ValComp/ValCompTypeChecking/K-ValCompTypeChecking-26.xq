(:*******************************************************:)
(: Test: K-ValCompTypeChecking-26                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Value comparison involving xs:untypedAtomic, which leads to an inexistent operator mapping. :)
(:*******************************************************:)
xs:untypedAtomic("3") eq 3
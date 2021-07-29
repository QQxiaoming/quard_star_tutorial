(:*******************************************************:)
(: Test: K2-TimeAddDTD-1                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: No '+' operator is available between xs:time and xs:time. :)
(:*******************************************************:)
xs:time("10:10:10") +
                      xs:time("23:10:10")
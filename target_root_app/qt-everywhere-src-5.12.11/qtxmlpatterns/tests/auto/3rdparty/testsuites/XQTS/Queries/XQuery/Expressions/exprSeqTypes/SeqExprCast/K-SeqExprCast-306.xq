(:*******************************************************:)
(: Test: K-SeqExprCast-306                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure casting xs:gMonthDay to xs:string, with timezone 'Z' is properly handled. :)
(:*******************************************************:)
xs:string(xs:gMonthDay("--01-01Z")) eq "--01-01Z"
(:*******************************************************:)
(: Test: K-SeqExprCast-284                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure casting xs:gYearMonth to xs:string, with timezone '+00:00' is properly handled. :)
(:*******************************************************:)
xs:string(xs:gYearMonth("1999-01+00:00")) eq "1999-01Z"
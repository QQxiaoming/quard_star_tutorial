(:*******************************************************:)
(: Test: K-SeqExprCast-365                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure casting xs:time to xs:string, with timezone '-00:00' is properly handled. :)
(:*******************************************************:)
xs:string(xs:time("23:59:12.999-00:00")) eq "23:59:12.999Z"
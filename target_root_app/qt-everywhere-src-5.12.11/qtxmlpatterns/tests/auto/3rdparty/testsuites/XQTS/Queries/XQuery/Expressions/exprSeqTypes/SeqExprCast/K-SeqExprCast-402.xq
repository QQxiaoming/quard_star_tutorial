(:*******************************************************:)
(: Test: K-SeqExprCast-402                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure casting xs:date to xs:string, with timezone '+00:00' is properly handled. :)
(:*******************************************************:)
xs:string(xs:date("1999-12-01+00:00")) eq "1999-12-01Z"
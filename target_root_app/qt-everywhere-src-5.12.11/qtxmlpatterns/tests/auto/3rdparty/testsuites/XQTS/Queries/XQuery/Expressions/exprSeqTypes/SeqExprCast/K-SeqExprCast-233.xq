(:*******************************************************:)
(: Test: K-SeqExprCast-233                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure casting xs:gYear to xs:string, with timezone 'Z' is properly handled. :)
(:*******************************************************:)
xs:string(xs:gYear("1999Z")) eq "1999Z"
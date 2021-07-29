(:*******************************************************:)
(: Test: K-SeqExprCast-114                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: An empty string is a valid lexical representation for xs:hexBinary, and means 'no data. :)
(:*******************************************************:)
xs:string(xs:hexBinary("")) eq ""
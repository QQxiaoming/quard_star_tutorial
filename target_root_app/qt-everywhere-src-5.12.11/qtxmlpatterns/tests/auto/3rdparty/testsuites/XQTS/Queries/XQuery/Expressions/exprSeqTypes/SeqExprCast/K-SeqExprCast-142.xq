(:*******************************************************:)
(: Test: K-SeqExprCast-142                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: An empty string is a valid lexical representation for xs:base64Binary, and means 'no data. :)
(:*******************************************************:)
xs:string(xs:base64Binary("")) eq ""
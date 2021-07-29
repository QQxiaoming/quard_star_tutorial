(:*******************************************************:)
(: Test: K-SeqExprCast-1357                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: 'castable as' involving xs:hexBinary as source type and xs:base64Binary as target type should always evaluate to true. :)
(:*******************************************************:)
xs:hexBinary("0FB7") castable as xs:base64Binary
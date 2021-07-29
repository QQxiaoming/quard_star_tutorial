(:*******************************************************:)
(: Test: K-SeqExprCast-1223                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: 'castable as' involving xs:boolean as source type and xs:double as target type should always evaluate to true. :)
(:*******************************************************:)
xs:boolean("true") castable as xs:double
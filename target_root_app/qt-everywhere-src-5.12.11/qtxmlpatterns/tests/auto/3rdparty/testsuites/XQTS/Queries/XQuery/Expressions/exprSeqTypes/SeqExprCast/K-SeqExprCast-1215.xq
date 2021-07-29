(:*******************************************************:)
(: Test: K-SeqExprCast-1215                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Applying fn:boolean() to a value of type xs:boolean that has the lexical value ' "true" ' should result in the boolean value true. :)
(:*******************************************************:)

          boolean(xs:boolean("true"))
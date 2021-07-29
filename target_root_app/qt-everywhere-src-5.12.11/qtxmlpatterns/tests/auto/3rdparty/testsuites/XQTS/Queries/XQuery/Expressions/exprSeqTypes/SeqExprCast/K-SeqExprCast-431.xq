(:*******************************************************:)
(: Test: K-SeqExprCast-431                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Applying fn:boolean() to a value of type xs:string that has the lexical value ' "an arbitrary string" ' should result in the boolean value true. :)
(:*******************************************************:)

          boolean(xs:string("an arbitrary string"))
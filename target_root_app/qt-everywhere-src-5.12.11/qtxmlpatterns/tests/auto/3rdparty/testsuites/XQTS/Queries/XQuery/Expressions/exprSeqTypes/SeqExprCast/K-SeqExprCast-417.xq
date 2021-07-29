(:*******************************************************:)
(: Test: K-SeqExprCast-417                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Applying fn:boolean() to a value of type xs:untypedAtomic that has the lexical value ' "an arbitrary string(untypedAtomic source)" ' should result in the boolean value true. :)
(:*******************************************************:)

          boolean(xs:untypedAtomic("an arbitrary string(untypedAtomic source)"))
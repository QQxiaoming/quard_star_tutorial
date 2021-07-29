(:*******************************************************:)
(: Test: K-sequenceExprTypeswitch-3                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: typeswitch test: A string literal is of type xs:string, even though it can be promoted to xs:untypedAtomic. :)
(:*******************************************************:)

		(typeswitch("a string")
			case xs:untypedAtomic return -1
			case xs:string return 1
			default return -2)
		eq 1
	
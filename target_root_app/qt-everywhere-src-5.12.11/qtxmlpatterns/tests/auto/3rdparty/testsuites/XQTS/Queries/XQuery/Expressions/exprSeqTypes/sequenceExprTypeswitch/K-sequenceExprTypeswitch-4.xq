(:*******************************************************:)
(: Test: K-sequenceExprTypeswitch-4                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A sequence of atomic items are not treated individually, but as a whole. :)
(:*******************************************************:)

		(typeswitch((1, "a string"))
			case xs:integer return -1
			case xs:string return -2
			case xs:anyAtomicType+ return 1
			default return -3)
		eq 1
	
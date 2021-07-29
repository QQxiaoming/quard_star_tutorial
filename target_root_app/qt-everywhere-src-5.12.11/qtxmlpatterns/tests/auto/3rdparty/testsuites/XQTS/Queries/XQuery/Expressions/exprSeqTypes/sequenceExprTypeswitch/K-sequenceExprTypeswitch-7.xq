(:*******************************************************:)
(: Test: K-sequenceExprTypeswitch-7                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A typeswitch with an operand expression being the comma operator using no paranteses. :)
(:*******************************************************:)

		(typeswitch(1, 2, 3)
			case xs:string+ return -1
			case xs:integer+ return 1
			default return -2)
		eq 1
	
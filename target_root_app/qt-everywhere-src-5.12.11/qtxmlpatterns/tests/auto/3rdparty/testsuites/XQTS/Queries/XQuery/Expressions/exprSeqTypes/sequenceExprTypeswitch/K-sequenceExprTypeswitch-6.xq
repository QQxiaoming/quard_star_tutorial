(:*******************************************************:)
(: Test: K-sequenceExprTypeswitch-6                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A typeswitch scenario involving empty-sequence(). Both the 'xs:integer*' branch and the 'empty-sequnec()' branch are valid. :)
(:*******************************************************:)

		(typeswitch(())
			case xs:integer* return 1
			case empty-sequence() return 1
			default return -2)
		eq 1
	
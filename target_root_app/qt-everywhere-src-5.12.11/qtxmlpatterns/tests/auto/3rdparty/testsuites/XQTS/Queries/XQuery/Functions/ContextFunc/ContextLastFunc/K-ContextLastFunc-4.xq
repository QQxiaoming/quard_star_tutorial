(:*******************************************************:)
(: Test: K-ContextLastFunc-4                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: A test whose essence is: `deep-equal((1, 2, 3)[last() eq last()], (1, 2, 3))`. :)
(:*******************************************************:)
deep-equal((1, 2, 3)[last() eq last()],
						 (1, 2, 3))
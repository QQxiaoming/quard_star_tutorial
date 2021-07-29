(:*******************************************************:)
(: Test: K-EmptyOrderProlog-3                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Two 'declare default order empty' declarations are invalid. :)
(:*******************************************************:)

	(::)declare(::)default order empty(::)greatest(::);
	(::)declare(::)default order empty(::)least(::); 1 eq 1
	
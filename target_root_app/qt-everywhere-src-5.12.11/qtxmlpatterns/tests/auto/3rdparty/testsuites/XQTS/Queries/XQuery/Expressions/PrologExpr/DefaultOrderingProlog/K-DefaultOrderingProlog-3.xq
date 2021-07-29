(:*******************************************************:)
(: Test: K-DefaultOrderingProlog-3                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Two 'declare ordering mode' declarations are invalid. :)
(:*******************************************************:)

	(::)declare(::)ordering (::)unordered(::);
	(::)declare(::)ordering (::)ordered(::); 1 eq 1
	
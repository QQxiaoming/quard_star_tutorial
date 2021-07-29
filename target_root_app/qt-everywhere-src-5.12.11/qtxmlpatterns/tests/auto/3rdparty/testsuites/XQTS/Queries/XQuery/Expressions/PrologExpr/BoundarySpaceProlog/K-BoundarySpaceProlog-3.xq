(:*******************************************************:)
(: Test: K-BoundarySpaceProlog-3                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Two 'declare boundary-space' declarations are invalid. :)
(:*******************************************************:)

	(::)declare(::)boundary-space(::)strip(::);
	(::)declare(::)boundary-space(::)preserve(::); 1 eq 1
	
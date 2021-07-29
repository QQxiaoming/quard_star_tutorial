(:*******************************************************:)
(: Test: K-CopyNamespacesProlog-4                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A 'declare copy-namespaces' declaration specifying no-preserve and no-inherit in a wrong order . :)
(:*******************************************************:)

		declare copy-namespaces no-inherit, no-preserve; 
		1 eq 1
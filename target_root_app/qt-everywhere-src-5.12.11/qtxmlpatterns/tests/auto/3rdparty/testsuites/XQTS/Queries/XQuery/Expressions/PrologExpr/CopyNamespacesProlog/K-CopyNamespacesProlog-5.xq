(:*******************************************************:)
(: Test: K-CopyNamespacesProlog-5                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A 'declare copy-namespaces' declaration specifying preserve and inherit in a wrong order . :)
(:*******************************************************:)

		declare copy-namespaces inherit, preserve; 
		1 eq 1
(:*******************************************************:)
(: Test: K-CopyNamespacesProlog-1                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A prolog may not contain two copy-namespaces declarations. :)
(:*******************************************************:)

		declare copy-namespaces preserve, no-inherit; 
		declare copy-namespaces no-preserve, no-inherit; 
		1 eq 1
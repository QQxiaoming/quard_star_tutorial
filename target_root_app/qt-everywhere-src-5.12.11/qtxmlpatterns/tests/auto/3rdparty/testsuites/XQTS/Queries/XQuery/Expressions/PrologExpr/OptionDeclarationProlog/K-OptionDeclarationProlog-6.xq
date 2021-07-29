(:*******************************************************:)
(: Test: K-OptionDeclarationProlog-6                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A syntactically invalid option declaration.  :)
(:*******************************************************:)
declare option localpartmissing: 'option value'; 1 eq 1
	
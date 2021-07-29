(:*******************************************************:)
(: Test: K-OptionDeclarationProlog-5                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: An undeclared prefix in a option declaration is an error regardless of any option support in the implementation. :)
(:*******************************************************:)
declare option prefixnotdeclared:opt 'option value'; 1 eq 1
	
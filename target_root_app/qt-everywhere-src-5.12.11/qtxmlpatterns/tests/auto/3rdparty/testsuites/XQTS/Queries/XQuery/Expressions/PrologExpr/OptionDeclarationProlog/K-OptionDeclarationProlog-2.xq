(:*******************************************************:)
(: Test: K-OptionDeclarationProlog-2                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Five identical options appearing after each other. :)
(:*******************************************************:)
declare(::)option(::)local:opt(::)"option value"(::);
		 declare(::)option(::)local:opt(::)"option value"(::);
		 declare(::)option(::)local:opt(::)"option value"(::);
		 declare(::)option(::)local:opt(::)"option value"(::);
		 declare(::)option(::)local:opt(::)"option value"(::);(::)1(::)eq(::)1
	
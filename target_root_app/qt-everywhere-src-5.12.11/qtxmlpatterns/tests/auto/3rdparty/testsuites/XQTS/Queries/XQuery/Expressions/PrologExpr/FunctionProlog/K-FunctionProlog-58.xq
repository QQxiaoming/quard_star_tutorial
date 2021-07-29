(:*******************************************************:)
(: Test: K-FunctionProlog-58                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: The type 'none()' isn't available to users.  :)
(:*******************************************************:)

declare function local:error() as none()
{
	1
};
local:error()

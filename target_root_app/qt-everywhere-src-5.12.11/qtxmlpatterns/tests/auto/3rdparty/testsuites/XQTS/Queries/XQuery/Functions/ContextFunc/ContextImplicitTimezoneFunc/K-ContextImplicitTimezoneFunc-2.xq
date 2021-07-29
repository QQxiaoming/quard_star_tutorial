(:*******************************************************:)
(: Test: K-ContextImplicitTimezoneFunc-2                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: Simple test of implicit-timezone().          :)
(:*******************************************************:)
seconds-from-duration(implicit-timezone()) le 0
				   or
				   seconds-from-duration(implicit-timezone()) gt 0
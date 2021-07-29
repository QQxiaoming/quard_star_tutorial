(:*******************************************************:)
(: Test: K2-AbbrAxes-1                                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Focus is undefined inside user functions; '..' axis. :)
(:*******************************************************:)
declare function local:myFunc()
                  {
                    ..
                  };
                  local:myFunc()
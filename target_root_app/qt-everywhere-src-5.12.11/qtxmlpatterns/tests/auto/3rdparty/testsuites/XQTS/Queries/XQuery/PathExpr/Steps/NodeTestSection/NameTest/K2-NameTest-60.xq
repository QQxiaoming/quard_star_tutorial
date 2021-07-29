(:*******************************************************:)
(: Test: K2-NameTest-60                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Combine the descendant-or-self axis with a processing-instruction test. :)
(:*******************************************************:)
declare variable $input := <e/>;
    empty(for $PI as processing-instruction() in $input//processing-instruction() return $PI)
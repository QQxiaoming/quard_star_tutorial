(:*******************************************************:)
(: Test: K2-NameTest-63                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Combine the descendant-or-self axis with the child axis. :)
(:*******************************************************:)
<x>
              <x>
                  <y id="0"/>
              </x>
                <y id="1"/>
       </x>/descendant-or-self::x/child::y
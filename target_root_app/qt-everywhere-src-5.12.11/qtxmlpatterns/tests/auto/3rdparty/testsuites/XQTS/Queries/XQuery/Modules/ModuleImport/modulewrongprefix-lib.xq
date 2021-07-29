(:*******************************************************:)
(: Test: modulewrongprefix-lib.xq                        :)
(: Written By: Carmelo Montanez                          :)
(: Date: 2006/03/21                                      :)
(: Purpose: Imported module with wrong namespace prefix. :)
(:*******************************************************:)

module namespace xml="http://www.w3.org/TestModules/test1";

declare variable $xml:flag := 1;
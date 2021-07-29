xquery version "1.1";

(:***********************************************************:)
(: Test: module-pub-priv.xq                                  :)
(: Written By: Carmelo Montanez                              :)
(: Date: 2009-10-28                                          :)
(: Purpose: Library module with public and private functions :)
(:***********************************************************:)

module namespace mod="http://www.w3.org/TestModules/module-pub-priv";

declare private function mod:f() { 23 };

declare public function mod:g($a as xs:integer) {
   mod:f() + $a
};
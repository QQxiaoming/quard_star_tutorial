(:**********************************************************:)
(:Test: validate-constraints-4.xq                           :)
(:Written By: Andrew Eisenberg                              :)
(:Purpose: Test validation of elements with unique/id/idref :)
(:         schema constraints                               :)
(:**********************************************************:)

import schema namespace constraints = "http://www.w3.org/constraints"; 

declare variable $test := element test { 
    <constraints:a>
        <b id="x"/>
        <b id="y"/>
    </constraints:a>
  };


validate { $test/*:a }  

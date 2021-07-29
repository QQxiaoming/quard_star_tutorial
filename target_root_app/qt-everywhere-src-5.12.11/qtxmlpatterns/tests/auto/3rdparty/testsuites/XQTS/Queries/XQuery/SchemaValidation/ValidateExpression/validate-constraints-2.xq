(:**********************************************************:)
(:Test: validate-constraints-2.xq                           :)
(:Written By: Nicolae Brinza                                :)
(:Purpose: Test validation of elements with unique/id/idref :)
(:         schema constraints                               :)
(:**********************************************************:)

import schema namespace constraints = "http://www.w3.org/constraints";

declare variable $test := element test { 
    <constraints:a2>
        <b2 id="x"/>
        <b2 id="y"/>
        <b2/>
    </constraints:a2>
  };


validate { $test/*:a2 }  

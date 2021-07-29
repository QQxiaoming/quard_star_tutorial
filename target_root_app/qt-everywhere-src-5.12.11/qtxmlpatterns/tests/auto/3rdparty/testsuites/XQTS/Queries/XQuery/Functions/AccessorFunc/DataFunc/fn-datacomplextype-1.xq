(:*******************************************************:)
(:Test: fn-datacomplextype-1                             :)
(:Written By: Nicolae Brinza                             :)
(:Purpose: Evaluates the "data" function                 :)
(:         with the argument set to a complex element    :)
(:*******************************************************:)

import schema namespace mc ="http://www.w3.org/XQueryTest/mixedcontent";

fn:data(
  validate {
    <mc:weather>
      <mc:temperature>30</mc:temperature>
      <mc:precipitation>10</mc:precipitation>
    </mc:weather>
  }
)


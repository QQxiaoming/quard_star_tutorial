(:*******************************************************:)
(:Test: op-subtract-dates-yielding-DTD-10                :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-dates-yielding-DTD" operator used  :)
(:together with an "or" expression.                      :)
(:*******************************************************:)
 
fn:string((xs:date("1985-07-05Z") - xs:date("1977-12-02Z"))) or fn:string((xs:date("1985-07-05Z") - xs:date("1960-11-07Z")))
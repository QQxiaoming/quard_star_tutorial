(:*******************************************************:)
(:Test: op-subtract-dates-yielding-DTD-9                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-dates-yielding-DTD" operator used  :)
(:together with an "and" expression.                      :)
(:*******************************************************:)
 
fn:string((xs:date("1993-12-09Z") - xs:date("1992-10-02Z"))) and fn:string((xs:date("1993-12-09Z") - xs:date("1980-10-20Z")))
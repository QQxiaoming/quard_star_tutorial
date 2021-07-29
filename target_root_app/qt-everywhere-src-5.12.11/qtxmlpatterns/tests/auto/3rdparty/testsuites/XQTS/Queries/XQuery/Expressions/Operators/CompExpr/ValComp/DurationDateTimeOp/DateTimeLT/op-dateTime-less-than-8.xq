(:*******************************************************:)
(:Test: op-dateTime-less-than-8                          :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dateTime-less-than" function used  :)
(:together with "and" expression (le operator).          :)
(:*******************************************************:)
 
(xs:dateTime("2002-04-02T12:00:00Z") le xs:dateTime("2005-04-02T12:00:20Z")) and (xs:dateTime("2002-04-02T12:10:00Z") le xs:dateTime("2002-04-03T12:00:00Z"))
(: Name: Constr-ws-tag-5 :)
(: Written by: Andreas Behm :)
(: Description: count text nodes when stripping whitespace :)

declare boundary-space strip;
fn:count((<elem>   	
    		      <a>                   

	
        </a>	
<a>        	     </a>  	           <a>  <b>

      		    </b>
	
  </a>
	

      </elem>)//text())
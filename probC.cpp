#include<bits/stdc++.h>
#define s(n)                        scanf("%d",&n)
#define sc(n)                       scanf("%c",&n)
#define sl(n)                       scanf("%lld",&n)
#define sf(n)                       scanf("%lf",&n)
#define ss(n)                       scanf("%s",n)
#define fill(a,v)                    memset(a, v, sizeof a)
#define sz(a)                       sizeof(a)/sizeof()

// Some common useful functions
#define maxi(a,b)                     ( (a) > (b) ? (a) : (b))
#define mini(a,b)                     ( (a) < (b) ? (a) : (b))
#define checkbit(n,b)                ( (n >> b) & 1)
#define INDEX(arr,ind)               (lower_bound(all(arr),ind)-arr.begin())

#define mod 1000000007
#define int_inf  1e8
#define long_inf 1e18

#define fr freopen("B-large.in","r",stdin)
#define fo freopen("output.txt","w",stdout)
#define print1(cnt,x) cout<<"Case #"<<cnt++<<": "<<x<<"\n";   // single-line o/p testcases
#define print2(cnt) cout<<"Case #"<<cnt++<<":"<<"\n";         // multiple-line o/p testcases
#define sit set<ull>::iterator
#define mit map<string,ull>::iterator

using namespace std;

typedef unsigned int ui;
typedef long long ll;
typedef unsigned long long ull;
typedef long double ld;

string convert(string key){

	string ans = "";
	int len = key.size();

	for(ull i=0; i<len; i++){
		ans = ans + (char)(((key[i]-'0')%2)+'0');
	}

	return ans;
}


int main(){	

	ull t;
	cin >> t;

	map<string,ull> m;
	char op;
	string in; 

	while(t--){
		cin >> op;
		if(op == '+'){
			cin >> in;
			in = convert(in);
			if(m.find(in) == m.end()){
				m[in] = 1;
			}
			else{
				m[in] += 1;
			}
		}
		else if(op == '-'){
			cin >> in;
			in = convert(in);
			m[in] -= 1;
		}
		else{
			cin >> in;
			ull count = 0;
			ull len1 = in.size();
			string pattern = in;

			while(len1 <= 18){
				if(m.find(pattern) != m.end()){
					count += m[pattern];
				}
				pattern = "0" + pattern;
				len1++;				
			}

			ull i = 0;
			pattern = in;
			string temp;
			len1 = in.size();

			while(pattern[i] != '1' && i < len1){
				temp = pattern.substr(i+1,len1);
				if(m.find(temp) != m.end()){
					count += m[temp];
				}
				i++;
			}

			cout << count << endl;
		}		
	}
	
	return 0;
}

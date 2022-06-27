#pragma once
#include <iostream>
#include <fstream>
#include <random>
#include <SDL.h>
#include <array>
#include <list>
#include "lodepng.h"

//Didn't want to include windows header to do resource exe inclusion so just follow along with this page if you want to do this. 
//https://stackoverflow.com/questions/22151099/including-data-file-into-c-project
//or this
//https://stackoverflow.com/questions/48326287/is-there-a-cross-platform-way-to-embed-resources-in-a-binary-application-written



namespace Gore {
	template<typename TP>
	struct FreeElement {
		TP el;
		int next;
	};
	template<class T>
	class FreeList {
	private:
		std::vector<FreeElement<T>> items;
		int first_free;
	public:
		FreeList() { first_free = -1; };
		//returns index of insertion point
		int insert(T element) {
			if (first_free != -1) {
				const int index = first_free;
				first_free = items[first_free].next;
				items[index].el = element;
				return index;
			}
			FreeElement<T> f = { element, -1 };
			items.push_back(f);
			return int(items.size() - 1);
		}

		void erase(int n) {
			items[n].next = first_free;
			first_free = n;
		}
		void clear() {
			items.clear();
			first_free = -1;
		}
		T& operator[](int n) {
			return items[n].el;
		}

		int size() {
			return static_cast<int>(items.size());
		}
	};
	
	struct TexListMem {
		SDL_Texture* current;
		TexListMem* next;
		std::string name;
	};
	typedef TexListMem* texp;
	struct SpriteListMem {
		SDL_Surface* current;
		SpriteListMem* next;
		std::string name;
	};
	typedef SpriteListMem* spxp;
	struct Point {
		int x;
		int y;
	};
	//4 byte: x, 4 byte: y, 4 byte: color data; repeat through data;
	struct PixelTransform {
		char* data;
		size_t size;
		PixelTransform* next;
	};
	typedef PixelTransform* TrList;

	class DeltaTimer {
	private:
		Uint64 LAST = 0;
		Uint64 NOW = SDL_GetPerformanceCounter();
	public:
		double getDelta();
	};
	class Engine {
	public:
		//Texture lists
		static void insertTex(TexListMem*& tex, SDL_Texture* current, std::string name);
		static SDL_Texture* findTex(texp head, std::string name);
		//Surface lists
		static void insertSprite(SpriteListMem*& sp, SDL_Surface* surf, std::string name);
		static SDL_Surface* findSprite(spxp sp, std::string name);
		//pixel manipulation
		static void SetPixelSurface(SDL_Surface* surf, int* y, int* x, Uint32* pixel);
		static void SetPixelSurface(SDL_Surface* surf, int y, int x, Uint32 pixel);
		static void massSetPixelSurface(SDL_Surface* surf, std::vector<Point> points, Uint32 Pixel);
		static void massSetPixelSurface(SDL_Surface* surf, Point tp, Point bt, Uint32 Pixel);
		static bool massGetPixelSurface(SDL_Surface* surf, Point tp, Point bt, Uint32 pixel);
		static bool massGetPixelSurface(SDL_Surface* surf, std::vector<Point> points, Uint32 pixel);

		static Uint32 GetPixelSurface(SDL_Surface* surf, int* y, int* x);
		static void SetPixelSurfaceColorRGB(SDL_Surface* surf, int* y, int* x, SDL_Color* color);
		static void SetPixelSurfaceColorRGBA(SDL_Surface* surf, int* y, int* x, SDL_Color* color);
		static Uint32 ConvertColorToUint32RGB(SDL_Color color, SDL_PixelFormat* format);
		static Uint32 ConvertColorToUint32RGBA(SDL_Color color, SDL_PixelFormat* format);
		static void clearSurface(SDL_Surface* surf);
		//texture manipulation
		static void clearTexture(SDL_Texture* tex, int* pitch, int w, int h);
		static void SetPixelTexture(SDL_Texture* tex, int* y, int* x, Uint32* pixel, int* pitch);
		static Uint32 GetPixelTexture(SDL_Texture* tex, int* y, int* x, int* pitch);
		static void MassTextureSet(SDL_Texture* tex, int sy, int sx, int endx, int endy, Uint32* pixel, int* pitch);
		static void MassTextureSet(SDL_Texture* tex, std::vector<Point> points, std::vector<Uint32>colors, int* pitch);
		static bool MassTextureCheck(SDL_Texture* tex, int sy, int sx, int endx, int endy, Uint32* pixel, int* pitch);
		static bool MassTextureCheck(SDL_Texture* tex, std::vector<Point> points, Uint32* pixel, int* pitch);

		//image loading
		static SDL_Surface* loadPNG(std::string name, SDL_PixelFormatEnum format, int w, int h);
		static SDL_Surface* LoadBMP(const char* file, SDL_PixelFormatEnum format);
		static texp& loadTextureList(std::vector<std::string> names, std::vector<unsigned int> widths, std::vector<unsigned int> heights, SDL_PixelFormatEnum format, SDL_Renderer* rend, std::string filepath = "NULL");
		static spxp& loadSpriteList(std::vector<std::string> names, std::vector<unsigned int> widths, std::vector<unsigned int> heights, SDL_PixelFormatEnum format, std::string filepath = "NULL");
		//Text functions
		static void mapTextTextures(int start, texp& out, texp& input);
		static void drawText(SDL_Renderer* rend, texp& texthead, std::string text, int x, int y, int w, int h);
		//misc utility
		static Point raycast2DPixel(SDL_Surface* surf, int sx, int sy, float angle, int step);
		static SDL_Surface* createCircle(int w, int h, SDL_Color startcolor);
		static SDL_Surface* fillCircle(int w, int h, SDL_Color startcolor);
		static SDL_Surface* createBloom(int w, int h, SDL_Color startcolor, float magnitude);
		//memory related
		static char* serilizeStruct(char* ptr, int size);
		static void deserilizeStruct(char* dest, char* data, int size);
		//point system
		static bool* createPoints(SDL_Surface* surf);
		static TrList generatePixelTransforms(spxp& spritelist);
		static void switchTranformFrames(SDL_Surface* surf, TrList& frames, TrList& begin);
		static SDL_Surface* initTransformSurf(spxp& head);
		//misc
		static float trajX(float deg);
		//Takes in degrees return radians
		static float trajY(float deg);

	};

	//procedural animation system
	namespace Animate {
		//Forward Kinematics bone
		class FKBone {
		private:
			float length;
		public:
			bool forw = true;
			float angle;
			FKBone* forward;
			FKBone* backward;
			float x;
			float y;
			FKBone(float ia, float il, float ix, float iy) { angle = ia; length = il; x = ix; y = iy; forward = NULL; backward = NULL; }
			//need to calculate angle for each based on sum of parents angles
			float getEndX() {
				float tangle = angle;
				FKBone* t = backward;
				while (t != NULL) {
					tangle += t->angle;
					t = t->backward;
				}
				return x + std::cosf(tangle) * length;
			}
			float getEndY() {
				float tangle = angle;
				FKBone* t = backward;
				while (t != NULL) {
					tangle += t->angle;
					t = t->backward;
				}
				return y + std::sinf(tangle) * length;
			}

		};
		//class to easily initilize list of bones
		class FKLimb {
		public:
			std::vector<FKBone> bones;
			//put bones in order of where you want them to align in limb structure
			FKLimb(std::vector<FKBone> bs) {
				bones = bs;
				for (int i = 0; i < bones.size(); i++) {
					if (i - 1 >= 0) {
						bones[i].backward = &bones[i - 1];
					}
					else {
						bones[i].backward = NULL;
					}
					if (i + 1 < bones.size()) {
						bones[i].forward = &bones[i + 1];
					}
					else {
						bones[i].forward = NULL;
					}

				}
			}
			~FKLimb() {
				bones.clear();
			}
			//
			void update() {
				for (int i = 0; i < bones.size(); i++) {
					if (bones[i].backward != NULL) {
						bones[i].x = bones[i - 1].getEndX();
						bones[i].y = bones[i - 1].getEndY();
					}
				}
			}
			//handle timing yourself
			int animate(int index, float minang, float maxang, float ainc) {
				if (index > bones.size() - 1) {
					return -1;
				}
				//edit targeted angles in a set angle range	
				if (bones[index].forw) {
					bones[index].angle += ainc;
				}
				else {
					bones[index].angle -= ainc;
				}
				if (bones[index].angle <= minang) {
					bones[index].forw = true;
				}
				else if (bones[index].angle >= maxang) {
					bones[index].forw = false;
				}
				return 1;
			}
			void debugDraw(SDL_Renderer* rend) {
				for (int i = 0; i < bones.size(); i++) {
					SDL_SetRenderDrawColor(rend, 255, 100, 150, 0);
					SDL_RenderDrawLineF(rend, bones[i].x, bones[i].y, bones[i].getEndX(), bones[i].getEndY());
					SDL_SetRenderDrawColor(rend, 255, 0, 0, 0);
					SDL_Rect rect = { bones[i].x, bones[i].y, 5, 5 };
					SDL_RenderFillRect(rend, &rect);
				}
			}
		};
		//Inverse Kinematics bone
		class IKBone {
		private:
			float angle;
		public:
			float length;
			IKBone* backward = NULL;
			IKBone* forward = NULL;
			float x, y;
			IKBone(float ix, float iy, float il, float ia) { x = ix; y = iy; length = il; angle = ia; }
			float getEndX() {
				return x + std::cosf(angle) * length;
			}
			float getEndY() {
				return y + std::sinf(angle) * length;
			}
			void pointAt(float ix, float iy) {
				float dx = ix - x;
				float dy = iy - y;
				angle = std::atan2f(dy, dx);
			}
			void drag(float ix, float iy) {
				pointAt(ix, iy);
				x = ix - std::cosf(angle) * length;
				y = iy - std::sinf(angle) * length;
				if (backward != NULL) {
					backward->drag(x, y);
				}
			}
		};

		class IKLimb {
		private:
			float x;
			float y;
		public:
			std::vector<IKBone> bones;
			IKLimb(IKBone b, int num) {
				bones.push_back(b);
				x = b.x;
				y = b.y;
				for (int i = 0; i < num; i++) {
					b.x += b.length;
					bones.push_back(b);
				}
				for (int i = 0; i < bones.size(); i++) {
					if (i - 1 >= 0) {
						bones[i].backward = &bones[i - 1];
					}
					else {
						bones[i].backward = NULL;
					}
					if (i + 1 < bones.size()) {
						bones[i].forward = &bones[i + 1];
					}
					else {
						bones[i].forward = NULL;
					}

				}

			}
			IKLimb(std::vector<IKBone> bs) {
				if (bs.size() > 0) {
					x = bs[0].x;
					y = bs[0].y;
					bones = bs;
					for (int i = 0; i < bones.size(); i++) {
						if (i - 1 >= 0) {
							bones[i].backward = &bones[i - 1];
						}
						else {
							bones[i].backward = NULL;
						}
						if (i + 1 < bones.size()) {
							bones[i].forward = &bones[i + 1];
						}
						else {
							bones[i].forward = NULL;
						}

					}
				}
				else {
					x = 0;
					y = 0;
				}

			}
			~IKLimb() {
				bones.clear();
			}
			void drag(float ix, float iy) {
				if (bones.size() > 0) {
					bones[bones.size() - 1].drag(ix, iy);
				}
			}
			void update() {
				for (int i = 0; i < bones.size(); i++) {
					if (bones[i].backward != NULL) {
						bones[i].x = bones[i - 1].getEndX();
						bones[i].y = bones[i - 1].getEndY();
					}
					else {
						bones[i].x = x;
						bones[i].y = y;
					}
				}
			}
			void reach(float ix, float iy) {
				drag(ix, iy);
				update();
			}
			void debugDraw(SDL_Renderer* rend) {
				for (int i = 0; i < bones.size(); i++) {
					SDL_SetRenderDrawColor(rend, 255, 100, 150, 0);
					SDL_RenderDrawLineF(rend, bones[i].x, bones[i].y, bones[i].getEndX(), bones[i].getEndY());
					SDL_SetRenderDrawColor(rend, 255, 0, 0, 0);
					SDL_Rect rect = { bones[i].x, bones[i].y, 5, 5 };
					SDL_RenderFillRect(rend, &rect);
				}
			}
		};
	}
	class Particle {
	public:
		float x;
		float y;
		float trajx;
		float trajy;
		int rangehigh;
		int rangelow;
		double animtime = 0;
		double movetime = 0;
		bool erase;
		SDL_Rect rect;
		Gore::texp bhead;
		Gore::texp head;
		Uint8 alpha = 255;
	public:
		Particle() { rangehigh = 0; rangelow = 0; x = 0; y = 0; trajx = 0; trajy = 0; rect = { 0, 0, 0, 0 }; head = NULL; bhead = head; erase = false; }
		Particle(float cx, float cy, int rangel, int rangeh, SDL_Rect crect, Gore::texp list) { rangehigh = rangeh; rangelow = rangel; x = cx; y = cy; trajx = 0; trajy = 0; rect = crect; head = list; bhead = head; erase = false; };
		virtual void draw(SDL_Renderer* rend) {
			SDL_SetTextureAlphaMod(head->current, alpha);
			rect.x = x;
			rect.y = y;
			SDL_RenderCopy(rend, head->current, NULL, &rect);
			SDL_SetTextureAlphaMod(head->current, 0);
		}
		virtual void update(double* delta) {
			animtime += *delta;
			movetime += *delta;
			if (movetime > 0.05) {
				x += trajx;
				y += trajy;
				movetime = 0;
			}
			if (animtime > 0.1) {
				head = head->next;
				alpha -= 5;
				if (alpha <= 0) {
					erase = true;
					alpha = 0;
				}
				if (head == NULL) {
					head = bhead;
				}
				animtime = 0;
			}
		}
	};

	class Emitter {
	private:
		std::vector<Particle> particles;
		Particle* p;
	public:
		Emitter() { p = NULL; timetospawn = 0.1; }
		Emitter(Particle* par, double spawntime) { p = par; timetospawn = spawntime; }
		double ctime = 0;
		double timetospawn;
		virtual void spawnParticle() {
			p->trajx = cos(double(p->rangelow + (std::rand() % (p->rangehigh - p->rangelow + 1))) * M_PI / 180.0);
			p->trajy = sin(double(p->rangelow + (std::rand() % (p->rangehigh - p->rangelow + 1))) * M_PI / 180.0);
			particles.push_back(*p);
		}
		virtual void update(double* delta, SDL_Renderer* rend) {
			ctime += *delta;
			if (ctime > timetospawn) {
				spawnParticle();
				ctime = 0;
			}
			for (int i = 0; i < particles.size();) {
				particles[i].update(delta);
				particles[i].draw(rend);
				if (particles[i].erase) {
					particles.erase(particles.begin() + i);
				}
				else {
					i++;
				}
			}
		}
	};

	class Bounder {
	private:
	public:
		int x, y;
		int w, h;
		Bounder() { x = 0; y = 0; w = 50; h = 50; }
		Bounder(int ix, int iy, int iw, int ih) { x = ix; y = iy; w = iw; h = ih; }

		bool contains(float ix, float iy) {
			return!(ix < x || iy < y || ix >= (x + w) || iy >= (y + h));
		}
		bool contains(Bounder b) {
			//this may not run faster depending on system, but is faster on my AMD ryzen 3600
			bool ip = false;
			(b.x >= x) ? ip = true : ip = false;
			(b.x + b.w < x + w) ? ip = true : ip = false;
			(b.y >= y) ? ip = true : ip = false;
			(b.y + b.h < y + h) ? ip = true : ip = false;
			return ip;
		}
		bool overlaps(Bounder b) {
			bool ip = false;
			(x < b.x + b.w) ? ip = true : ip = false;
			(x + w >= b.x) ? ip = true : ip = false;
			(y < b.y + b.h) ? ip = true : ip = false;
			(y + h >= b.y) ? ip = true : ip = false;
			return ip;
		}
	};



	namespace SpatialAcceleration {


		template<typename TP>
		struct QuadElt {
			//actual data
			TP data;

			//bounder of element, not even needed
			Gore::Bounder b;
		};
		struct QuadEltNode {
			//index that points to next element
			int next;

			//index to element in element list
			int index;

			//if element needs to be moved
			bool move;
		};

		//instead of using array here use a single index which will be a "pointer" to a begin point in node vector, which we can then add +1, +2, +3, 
		//to get rest of elements
		struct QuadNode {
			//first index to child, -1 if no children
			int32_t child;

			//mumber of elements in node
			int32_t count;

			//used to calculate bounder based on depth of tree search
			Gore::Point p;

			//index to elt node, which begins this nodes elments
			int eltn_index;
		};

		template<class T>
		class QuadTree {
		public:
			//freelist containing the elements
			Gore::FreeList<QuadElt<T>> elts;
			//stores all element nodes
			Gore::FreeList<QuadEltNode> elt_nodes;

			//stores all nodes in tree, first node is always root
			std::vector<QuadNode> nodes;

			//size of root bounder
			Gore::Bounder root_rect;

			//index of first free node to be reclaimed as 4 contigous nodes at once. -1 is freelist empty
			int free_node;
			//the max depth
			int m_depth;
		public:
			QuadTree(int md, Gore::Bounder rt) {
				m_depth = md;
				//nodes are inserted 4 at a time use this to find free point where free
				free_node = -1;
				root_rect = rt;
				QuadNode no;
				no.p = { (int)rt.x, (int)rt.y };
				no.count = 0;
				no.eltn_index = -1;
				no.child = -1;
				nodes.push_back(no);
			}
			QuadNode* search(int node, Gore::Bounder b, int32_t dp) {
				//recursive search into nodes
				QuadNode* out = &nodes[node];
				QuadNode* nd = &nodes[node];
				Gore::Bounder tb(nd->p.x, nd->p.y, root_rect.w >> dp, root_rect.h >> dp);
				dp++;
				if (!tb.contains(b)) {
					return NULL;
				}
				if (dp + 1 <= m_depth) {
					Gore::Bounder bos(nd->p.x, nd->p.y, tb.w >> 1, tb.h >> 1);
					for (int i = 0; i < 4; i++) {
						switch (i) {
						case 1:
							bos = Gore::Bounder(nd->p.x + (tb.w >> 1), nd->p.y, tb.w >> 1, tb.h >> 1);
							break;
						case 2:
							bos = Gore::Bounder(nd->p.x, nd->p.y + (tb.h >> 1), tb.w >> 1, tb.h >> 1);
							break;
						case 3:
							bos = Gore::Bounder(nd->p.x + (tb.w >> 1), nd->p.y + (tb.h >> 1), tb.w >> 1, tb.h >> 1);
							break;
						}
						if (bos.contains(b)) {
							if (nd->child != -1) {
								out = &nodes[nd->child + i];
								QuadNode* tp = search(nd->child + i, b, dp);
								if (tp != NULL) {
									out = tp;
								}
							}
							else {
								//generate the node
								for (int j = 0; j < 4; j++) {
									QuadNode np;
									np.p = { bos.x, bos.y };
									np.child = -1;
									np.count = 0;
									np.eltn_index = -1;
									switch (j) {
									case 0:
										np.p.x = bos.x;
										np.p.y = bos.y;
										break;
									case 1:
										np.p.x = (bos.w > 0) ? bos.x + (bos.w >> 1) : 0;
										np.p.y = bos.y;
										break;
									case 2:
										np.p.x = bos.x;
										np.p.y = (bos.y > 0) ? bos.y + (bos.h >> 1) : 0;
										break;
									case 3:
										np.p.x = (bos.w > 0) ? bos.x + (bos.w >> 1) : 0;
										np.p.y = (bos.y > 0) ? bos.y + (bos.h >> 1) : 0;
										break;
									}
									nodes.push_back(np);
								}
								nodes[node].child = nodes.size() - 4;
								out = &nodes[nodes[node].child + i];
								QuadNode* tp = search(nodes[node].child + i, b, dp);
								if (tp != NULL) {
									out = tp;
								}
							}
							return out;
						}
					}
				}
				return out;
			}
			//insert element into tree
			void insert(T in, Gore::Bounder b) {
				QuadElt<T> nop = { in, b };
				QuadNode* nt = search(0, b, 0);
				if (nt != NULL) {
					//add into returned node
					QuadEltNode np;
					np.index = elts.insert(nop);
					np.next = nt->eltn_index;
					np.move = false;
					nt->eltn_index = elt_nodes.insert(np);
					nt->count++;
					return;
				}
				//add into root node
				QuadEltNode np;
				np.index = elts.insert(nop);
				np.next = nodes[0].eltn_index;
				np.move = false;
				nodes[0].eltn_index = elt_nodes.insert(np);
				nodes[0].count++;

			}
			//has memory leak issue
			void remove(int index, QuadNode* nt) {
				//nullify current elt index
				//try and remove any branching
				int der = elt_nodes[index].next;
				if (nt->count == 1) {
					nt->eltn_index = -1;
					elt_nodes.erase(index);
				}
				else if (der != -1) {
					QuadEltNode* npt = &elt_nodes[der];
					elt_nodes[index].index = npt->index;
					elt_nodes[index].next = npt->next;
					elt_nodes.erase(der);
				}
				else {
					//think this is creating problems with deleting zero
					elt_nodes[index].index = -1;
					elt_nodes[index].next = -1;
					//elt_nodes.erase(index);
				}
				nt->count--;
				if (nt->count <= 0) {
					nt->eltn_index = -1;
				}
			}
			void erase(int elt_index) {
				elts.erase(elt_index);
			}
			//this is broken
			void move(int n_index, int index) {
				//just move index pointer to different tree
				int elt_in = elt_nodes[index].index;
				remove(index, &nodes[n_index]);
				QuadNode* ntp = search(0, elts[elt_in].b, 0);
				if (ntp == NULL) {
					return;
				}
				QuadEltNode np;
				np.index = elt_in;
				np.next = ntp->eltn_index;
				np.move = false;
				ntp->eltn_index = elt_nodes.insert(np);
				ntp->count++;
			}
			//will move elts that need to be moved
			void clean_recur(int n_index) {
				int inp = nodes[n_index].eltn_index;
				while (inp != -1) {
					if (elt_nodes[inp].index != -1 && elt_nodes[inp].move) {
						move(n_index, inp);
					}
					inp = elt_nodes[inp].next;
				}
				int i = 0;
				(nodes[n_index].child != -1) ? i = 0 : i = 4;
				for (i; i < 4; i++) {
					clean_recur(nodes[n_index].child + i);
				}
			}

			void cleanup() {
				clean_recur(0);
			}

			int size() {
				return elts.size();
			}
			void searchNodes(QuadNode* node, Gore::Bounder b, std::vector<QuadNode*>& nds, size_t* depth) {
				if (b.contains(Gore::Bounder(node->p.x, node->p.y, root_rect.w >> *depth, root_rect.h >> *depth))) {
					(*depth)++;
					if (node->count > 0) {
						nds.push_back(node);
					}
					for (int i = 0; i < 4; i++) {
						if (nodes[node->child + i] != -1) {
							searchNodes(&nodes[node->child + i], b, nds, depth);
						}
					}
				}
			}

			std::vector<QuadNode*> getNodes(Gore::Bounder b) {
				//traverse tree and check if nodes are contained in checking area
				std::vector<QuadNode*> nds;
				size_t depth = 0;
				searchNodes(&nodes[0], b, nds, &depth);
				return nds;
			}
		};
	}
}
